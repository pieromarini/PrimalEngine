#include "fastgltf/types.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "vulkan_loader.h"
#include "stb_image.h"

#include <filesystem>
#include "vk_types.h"
#include "vulkan_renderer.h"
#include <glm/gtx/quaternion.hpp>


namespace pm {

std::optional<AllocatedImage> loadImage(VulkanRenderer* renderer, fastgltf::Asset& asset, fastgltf::Image& image) {
	AllocatedImage newImage{};

	int width{}, height{}, nrChannels{};

	std::visit(
		fastgltf::visitor{
			[](auto& arg) {},
			[&](fastgltf::sources::URI& filePath) {
				assert(filePath.fileByteOffset == 0);// We don't support offsets with stbi.
				assert(filePath.uri.isLocalPath());// We're only capable of loading
																					 // local files.

				const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());
				unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
				if (data) {
					VkExtent3D imagesize;
					imagesize.width = width;
					imagesize.height = height;
					imagesize.depth = 1;

					newImage = renderer->createImage(image.name.c_str(), data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);

					stbi_image_free(data);
				}
			},
			[&](fastgltf::sources::Vector& vector) {
				unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
				if (data) {
					VkExtent3D imagesize;
					imagesize.width = width;
					imagesize.height = height;
					imagesize.depth = 1;

					newImage = renderer->createImage(image.name.c_str(), data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);

					stbi_image_free(data);
				}
			},
			[&](fastgltf::sources::BufferView& view) {
				auto& bufferView = asset.bufferViews[view.bufferViewIndex];
				auto& buffer = asset.buffers[bufferView.bufferIndex];

				std::visit(fastgltf::visitor{ // We only care about VectorWithMime here, because we
																			// specify LoadExternalBuffers, meaning all buffers
																			// are already loaded into a vector.
										 [](auto& arg) {},
										 [&](fastgltf::sources::Vector& vector) {
											 unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
												 static_cast<int>(bufferView.byteLength),
												 &width,
												 &height,
												 &nrChannels,
												 4);
											 if (data) {
												 VkExtent3D imagesize;
												 imagesize.width = width;
												 imagesize.height = height;
												 imagesize.depth = 1;

												 newImage = renderer->createImage(image.name.c_str(), data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);

												 stbi_image_free(data);
											 }
										 } },
					buffer.data);
			},
		},
		image.data);

	// if any of the attempts to load the data failed, we havent written the image
	// so handle is null
	if (newImage.image == VK_NULL_HANDLE) {
		return {};
	} else {
		return newImage;
	}
}

VkFilter extractFilter(fastgltf::Filter filter) {
	switch (filter) {
	// nearest samplers
	case fastgltf::Filter::Nearest:
	case fastgltf::Filter::NearestMipMapNearest:
	case fastgltf::Filter::NearestMipMapLinear:
		return VK_FILTER_NEAREST;

	// linear samplers
	case fastgltf::Filter::Linear:
	case fastgltf::Filter::LinearMipMapNearest:
	case fastgltf::Filter::LinearMipMapLinear:
	default:
		return VK_FILTER_LINEAR;
	}
}

VkSamplerMipmapMode extractMipmapMode(fastgltf::Filter filter) {
	switch (filter) {
	case fastgltf::Filter::NearestMipMapNearest:
	case fastgltf::Filter::LinearMipMapNearest:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;

	case fastgltf::Filter::NearestMipMapLinear:
	case fastgltf::Filter::LinearMipMapLinear:
	default:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanRenderer* renderer, std::string_view filePath) {
	std::cout << std::format("Loading GLTF: {}", filePath) << '\n';
	if (!std::filesystem::exists(filePath)) {
		std::cout << std::format("Cannot load {}. File does not exist.\n", filePath);
		return {};
	}

	auto scene = std::make_shared<LoadedGLTF>();
	scene->renderer = renderer;
	LoadedGLTF& file = *scene.get();

	fastgltf::Parser parser{};

	constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble | fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;
	// fastgltf::Options::LoadExternalImages;

	fastgltf::GltfDataBuffer data;
	data.loadFromFile(filePath);

	fastgltf::Asset gltf;

	std::filesystem::path path = filePath;

	auto type = fastgltf::determineGltfFileType(&data);
	if (type == fastgltf::GltfType::glTF) {
		auto load = parser.loadGLTF(&data, path.parent_path(), gltfOptions);
		if (load) {
			gltf = std::move(load.get());
		} else {
			std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
			return {};
		}
	} else if (type == fastgltf::GltfType::GLB) {
		auto load = parser.loadBinaryGLTF(&data, path.parent_path(), gltfOptions);
		if (load) {
			gltf = std::move(load.get());
		} else {
			std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
			return {};
		}
	} else {
		std::cerr << "Failed to determine glTF container" << std::endl;
		return {};
	}

	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .ratio = 3 },
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 3 },
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 3 }
	};

	file.descriptorPool.init(renderer->m_device, gltf.materials.size(), sizes);
	for (fastgltf::Sampler& sampler : gltf.samplers) {
		VkSamplerCreateInfo samplerCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr
		};
		samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerCreateInfo.minLod = 0;

		samplerCreateInfo.magFilter = extractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
		samplerCreateInfo.minFilter = extractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

		samplerCreateInfo.mipmapMode = extractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

		VkSampler newSampler{};
		vkCreateSampler(renderer->m_device, &samplerCreateInfo, nullptr, &newSampler);

		file.samplers.push_back(newSampler);
	}

	// temporal arrays for all the objects to use while creating the GLTF data
	std::vector<std::shared_ptr<MeshAsset>> meshes;
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<AllocatedImage> images;
	std::vector<std::shared_ptr<GLTFMaterial>> materials;

	// load textures
	int defaultTextureCount = 0;
	for (fastgltf::Image& image : gltf.images) {
		auto img = loadImage(renderer, gltf, image);

		if (img.has_value()) {
			images.push_back(*img);
			file.images[image.name.c_str()] = *img;
		} else {
			images.push_back(renderer->errorCheckerboardImage);
			defaultTextureCount++;
			std::cout << "gltf failed to load texture " << image.name << std::endl;
		}
	}
	std::cout << std::format("Loaded {} textures. Errors: {}\n", images.size(), defaultTextureCount);


	// create buffer to hold the material data
	file.materialDataBuffer = renderer->createBuffer("materialDataBuffer", sizeof(GLTFMetallic_Roughness::MaterialConstants) * gltf.materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	int data_index = 0;
	auto sceneMaterialConstants = static_cast<GLTFMetallic_Roughness::MaterialConstants*>(file.materialDataBuffer.info.pMappedData);

	// Load materials
	for (fastgltf::Material& mat : gltf.materials) {
		auto newMat = std::make_shared<GLTFMaterial>();
		materials.push_back(newMat);
		file.materials[mat.name.c_str()] = newMat;
		newMat->name = mat.name;

		GLTFMetallic_Roughness::MaterialConstants constants{};
		constants.colorFactors.x = mat.pbrData.baseColorFactor[0];
		constants.colorFactors.y = mat.pbrData.baseColorFactor[1];
		constants.colorFactors.z = mat.pbrData.baseColorFactor[2];
		constants.colorFactors.w = mat.pbrData.baseColorFactor[3];

		constants.metalRoughFactors.x = mat.pbrData.metallicFactor;
		constants.metalRoughFactors.y = mat.pbrData.roughnessFactor;
		// write material parameters to buffer
		sceneMaterialConstants[data_index] = constants;

		MaterialPass passType = MaterialPass::MainColor;
		if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
			passType = MaterialPass::Transparent;
		} else if (mat.doubleSided) {
      passType = MaterialPass::DoubleSided;
    }

		GLTFMetallic_Roughness::MaterialResources materialResources{};

		// default the material textures
		materialResources.colorImage = renderer->whiteImage;
		materialResources.colorSampler = renderer->defaultSamplerLinear;
		materialResources.metalRoughImage = renderer->whiteImage;
		materialResources.metalRoughSampler = renderer->defaultSamplerLinear;

		// set the uniform buffer for the material data
		materialResources.dataBuffer = file.materialDataBuffer.buffer;
		materialResources.dataBufferOffset = data_index * sizeof(GLTFMetallic_Roughness::MaterialConstants);
		// grab textures from gltf file
		if (mat.pbrData.baseColorTexture.has_value()) {
			size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
			size_t sampler = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value();

			materialResources.colorImage = images[img];
			materialResources.colorSampler = file.samplers[sampler];
		}
		// build material
		newMat->data = renderer->metalRoughMaterial.writeMaterial(renderer->m_device, passType, materialResources, file.descriptorPool);

		data_index++;
	}

	// use the same vectors for all meshes so that the memory doesnt reallocate as often
	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	for (auto& mesh : gltf.meshes) {
		std::shared_ptr<MeshAsset> newmesh = std::make_shared<MeshAsset>();
		meshes.push_back(newmesh);
		file.meshes[mesh.name.c_str()] = newmesh;
		newmesh->name = mesh.name;

		for (auto&& p : mesh.primitives) {
			GeoSurface newSurface;
			auto vertexOffset = static_cast<int32_t>(vertices.size());

			newSurface.firstIndex = (uint32_t)indices.size();
			newSurface.indexCount = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;
			newSurface.vertexOffset = vertexOffset;

			// load indices
			{
				fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
				indices.reserve(indices.size() + indexaccessor.count);

				fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor, [&](std::uint32_t idx) {
					indices.push_back(idx);
				});
			}

			// load vertex positions
			{
				fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
				vertices.resize(vertices.size() + posAccessor.count);

				fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](glm::vec3 v, size_t index) {
					Vertex newvtx{};
					newvtx.position = v;
					newvtx.normal = { 1, 0, 0 };
					newvtx.color = glm::vec4{ 1.f };
					newvtx.uv_x = 0;
					newvtx.uv_y = 0;
					vertices[vertexOffset + index] = newvtx;
				});
			}

			// load vertex normals
			auto normals = p.findAttribute("NORMAL");
			if (normals != p.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second], [&](glm::vec3 v, size_t index) {
					vertices[vertexOffset + index].normal = v;
				});
			}

			// load UVs
			auto uv = p.findAttribute("TEXCOORD_0");
			if (uv != p.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second], [&](glm::vec2 v, size_t index) {
					vertices[vertexOffset + index].uv_x = v.x;
					vertices[vertexOffset + index].uv_y = v.y;
				});
			}

			// load vertex colors
			auto colors = p.findAttribute("COLOR_0");
			if (colors != p.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second], [&](glm::vec4 v, size_t index) {
					vertices[vertexOffset + index].color = v;
				});
			}

			// TODO: This can fail if the file doesn't have any materials.
			// We should have a "default" material as part of the engine
			if (p.materialIndex.has_value()) {
				newSurface.material = materials[p.materialIndex.value()];
			} else {
				newSurface.material = materials[0];
			}

			newmesh->surfaces.push_back(newSurface);
		}
	}
	// Upload all the vertex/index data for the loaded model
	
	file.modelBuffers = renderer->uploadMesh<Vertex>(indices, vertices);

	// load all nodes and their meshes
	for (fastgltf::Node& node : gltf.nodes) {
		std::shared_ptr<Node> newNode;

		// find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the meshnode class
		if (node.meshIndex.has_value()) {
			newNode = std::make_shared<MeshNode>();
			dynamic_cast<MeshNode*>(newNode.get())->mesh = meshes[*node.meshIndex];
		} else {
			newNode = std::make_shared<Node>();
		}

		nodes.push_back(newNode);
		file.nodes[node.name.c_str()];

		std::visit(fastgltf::visitor{
								 [&](fastgltf::Node::TransformMatrix matrix) {
									 memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
								 },
								 [&](fastgltf::Node::TRS transform) {
									 glm::vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
									 glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);
									 glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

									 glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
									 glm::mat4 rm = glm::toMat4(rot);
									 glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

									 newNode->localTransform = tm * rm * sm;
								 } },
			node.transform);
	}

	// Setup transform hierarchy
	for (int i = 0; i < gltf.nodes.size(); i++) {
		fastgltf::Node& node = gltf.nodes[i];
		std::shared_ptr<Node>& sceneNode = nodes[i];

		for (auto& c : node.children) {
			sceneNode->children.push_back(nodes[c]);
			nodes[c]->parent = sceneNode;
		}
	}

	// find the top nodes, with no parents
	for (auto& node : nodes) {
		if (node->parent.lock() == nullptr) {
			file.topNodes.push_back(node);
			node->refreshTransform(glm::mat4{ 1.f });
		}
	}

	return scene;
}

void LoadedGLTF::draw(const glm::mat4& topMatrix, DrawContext& ctx) {
	// Initialize draw context
	// TODO: It's weird having this here.
	for(const auto& [ materialName, material ]: materials) {
		ModelDrawRender modelDrawRender{ .modelBuffers = &modelBuffers, .material = &material->data };
		if (material->data.passType == MaterialPass::Transparent) {
			ctx.transparentDraws.emplace(materialName, modelDrawRender);
		} else {
			ctx.opaqueDraws.emplace(materialName, modelDrawRender);
		}
	}

	for (auto& n : topNodes) {
		n->draw(topMatrix, ctx);
	}
}

void LoadedGLTF::clearAll() {
	VkDevice dv = renderer->m_device;

	renderer->destroyBuffer(modelBuffers.indexBuffer);
	renderer->destroyBuffer(modelBuffers.vertexBuffer);

	for (auto& [k, v] : images) {
		if (v.image == renderer->errorCheckerboardImage.image) {
			// dont destroy the default images
			continue;
		}
		renderer->destroyImage(v);
	}

	for (auto& sampler : samplers) {
		vkDestroySampler(dv, sampler, nullptr);
	}

	descriptorPool.destroyPools(dv);
	renderer->destroyBuffer(materialDataBuffer);
}

}// namespace pm
