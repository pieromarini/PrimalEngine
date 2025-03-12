#include "fastgltf/types.hpp"
#include <vulkan/vulkan_core.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vulkan_loader.h"

#include "vk_types.h"
#include "vulkan_renderer.h"
#include <filesystem>
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
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 3 },
	};
	file.descriptorPool.init(renderer->m_device, gltf.materials.size() + 1, sizes);

	// Load default sampler
	file.samplers.push_back(renderer->defaultSamplerLinear);

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

	// Set default texture
	images.push_back(renderer->errorCheckerboardImage);
	file.images["DefaultTexture"] = renderer->errorCheckerboardImage;

	// load textures
	int defaultTextureCount = 0;
	for (fastgltf::Image& image : gltf.images) {
		auto img = loadImage(renderer, gltf, image);
		if (img.has_value()) {
			if (image.name == "Vespa_BaseColor-Vespa_BaseColor") {
				std::cout << "Loading Vespa_BaseColor-Vespa_BaseColor at index: " << images.size() << '\n';
			} else if (image.name == "Vespa_Odometer_BaseColor-Vespa_Odometer_BaseColor") {
				std::cout << "Loading Vespa_Odometer_BaseColor-Vespa_Odometer_BaseColor at index: " << images.size() << '\n';
			}

			images.push_back(*img);
			file.images[image.name.c_str()] = *img;
		} else {
			images.push_back(renderer->errorCheckerboardImage);
			defaultTextureCount++;
			std::cout << "gltf failed to load texture " << image.name << '\n';
		}
	}
	std::cout << std::format("Loaded {} textures. Errors: {}\n", images.size(), defaultTextureCount);


	// TODO: We store the data in a vector but also write directly to the buffer.
	// 			 We don't need both and it's wasteful. We should refactor this.
	renderer->globalMaterialData.reserve(gltf.materials.size() + 1);
	renderer->globalMaterialDataBuffer = renderer->createBuffer("globalMaterialDataBuffer", sizeof(MaterialData) * (gltf.materials.size() + 1), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	auto sceneMaterialData = static_cast<MaterialData*>(renderer->globalMaterialDataBuffer.info.pMappedData);

	// Create default material
	MaterialData defaultMaterialData{
		.albedoTexture = 0,
		.normalTexture = 0,
		.specularTexture = 0,
		.emissiveTexture = 0,
		.colorFactors = { 0.85f, 0.0f, 1.0f, 1.0f },
		.metalRoughFactors = { 0.0f, 0.0f, 0.0f, 0.0f }
	};
	renderer->globalMaterialData.push_back(defaultMaterialData);
	sceneMaterialData[0] = defaultMaterialData;

	auto defaultMat = std::make_shared<GLTFMaterial>();
	materials.push_back(defaultMat);
	file.materials["DefaultMaterial"] = defaultMat;
	defaultMat->name = "DefaultMaterial";

	// Write default texture to descriptor set and set Material pass and pipeline
	renderer->metalRoughMaterial.writeBindlessTextureToGlobalDescriptor(renderer->m_device, renderer->bindlessTexturesDescriptorSet, images[0], file.samplers[0], 0);
	defaultMat->data = renderer->metalRoughMaterial.writeMaterials(renderer->m_device, MaterialPass::MainColor, file.descriptorPool);

	// Load materials
	int materialDataIndex = 1;
	uint32_t bindlessTextureIndex = 1;
	for (fastgltf::Material& mat : gltf.materials) {
		auto newMat = std::make_shared<GLTFMaterial>();
		materials.push_back(newMat);
		file.materials[mat.name.c_str()] = newMat;
		newMat->name = mat.name;

		MaterialData materialData{};
		materialData.colorFactors.x = mat.pbrData.baseColorFactor[0];
		materialData.colorFactors.y = mat.pbrData.baseColorFactor[1];
		materialData.colorFactors.z = mat.pbrData.baseColorFactor[2];
		materialData.colorFactors.w = mat.pbrData.baseColorFactor[3];

		materialData.metalRoughFactors.x = mat.pbrData.metallicFactor;
		materialData.metalRoughFactors.y = mat.pbrData.roughnessFactor;

		MaterialPass passType = MaterialPass::MainColor;
		if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
			passType = MaterialPass::Transparent;
		} else if (mat.doubleSided) {
			passType = MaterialPass::DoubleSided;
		}

		// default the material textures
		// materialResources.colorImage = renderer->whiteImage;
		// materialResources.colorSampler = renderer->defaultSamplerLinear;
		// materialResources.metalRoughImage = renderer->whiteImage;
		// materialResources.metalRoughSampler = renderer->defaultSamplerLinear;

		// Set textures to "Default"
		materialData.albedoTexture = 0;
		materialData.normalTexture = 0;
		materialData.specularTexture = 0;
		materialData.emissiveTexture = 0;

		// grab textures from gltf file
		// TODO: Set rest of the textures
		if (mat.pbrData.baseColorTexture.has_value()) {
			auto textureIndex = mat.pbrData.baseColorTexture.value().textureIndex;
			size_t img = gltf.textures[textureIndex].imageIndex.value() + 1;
			size_t sampler = gltf.textures[textureIndex].samplerIndex.value() + 1;
			if (mat.name == "Vespa") {
				std::cout << "Vespa Texture index: " << textureIndex << " img: " << img << '\n';
			} else if (mat.name == "Vespa_Odometer") {
				std::cout << "Vespa_Odometer Texture index: " << textureIndex << " img: " << img << '\n';
			}

			materialData.albedoTexture = bindlessTextureIndex;
			// TODO: We are writing textures 1 by 1. We should batch these.
			renderer->metalRoughMaterial.writeBindlessTextureToGlobalDescriptor(renderer->m_device, renderer->bindlessTexturesDescriptorSet, images[img], file.samplers[sampler], bindlessTextureIndex);
			bindlessTextureIndex++;
		}

		// write material data to buffer
		sceneMaterialData[materialDataIndex] = materialData;
		renderer->globalMaterialData.push_back(materialData);

		// build material
		newMat->data = renderer->metalRoughMaterial.writeMaterials(renderer->m_device, passType, file.descriptorPool);

		materialDataIndex++;
	}
	std::cout << std::format("Loaded {} materials\n", materials.size());

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

			if (p.materialIndex.has_value()) {
				newSurface.material = materials[p.materialIndex.value() + 1];
				newSurface.material->data.materialIndex = p.materialIndex.value() + 1;// set material index to reference in shader
			} else {
				newSurface.material = materials[0];// set default material
				newSurface.material->data.materialIndex = 0;
			}

			if (mesh.name == "Bistro_Research_Exterior__lod0_Vespa_3937") {
				std::cout << "Loading Bistro_Research_Exterior__lod0_Vespa_3937 with material index: " << p.materialIndex.value() << '\n';
			}

			newmesh->surfaces.push_back(newSurface);
		}
	}

	// Upload all the vertex/index data for the loaded model
	file.modelBuffers = renderer->uploadMesh<Vertex>(indices, vertices, "modelBuffers");

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
	// TODO: This is just a hack for now. We should group materials by Pipeline.
	// 			 Right now we only have 2 pipelines: one for Opaques and one for alpha-blended (we should have one more for transparent objects)
	ctx.modelBuffers = &modelBuffers;
	for (const auto& [materialName, material] : materials) {
		ModelDrawRender modelDrawRender{ .material = &material->data };
		if (material->data.passType == MaterialPass::Transparent) {
			ctx.transparentDraws = modelDrawRender;
		} else {
			ctx.opaqueDraws = modelDrawRender;
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
	// TODO: should be handled by the renderer
	renderer->destroyBuffer(renderer->globalMaterialDataBuffer);
}

}// namespace pm
