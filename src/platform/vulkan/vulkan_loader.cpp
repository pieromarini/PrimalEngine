#include "assets/image_loader.h"
#include "entity.h"
#include "fastgltf/parser.hpp"
#include "fastgltf/types.hpp"
#include "material.h"
#include <ratio>
#include <vulkan/vulkan_core.h>
#include "vulkan_loader.h"

#include "vk_types.h"
#include "vulkan_renderer.h"
#include <filesystem>
#include <glm/gtx/quaternion.hpp>

#include "geometry.h"


namespace pm {

std::optional<AllocatedImage> loadImage(VulkanRenderer* renderer, fastgltf::Asset& asset, fastgltf::Image& image) {
	AllocatedImage newImage{};

	int width{}, height{}, nrChannels{};

	std::visit(
		fastgltf::visitor{
			[](auto& arg) {},
			[&](fastgltf::sources::URI& filePath) {
				assert(filePath.fileByteOffset == 0);// We don't support offsets with stbi.
				assert(filePath.uri.isLocalPath());// We're only capable of loading local files.

				const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());

				if (filePath.mimeType == fastgltf::MimeType::KTX2) {
					// TODO
				} else {
					auto imageAsset = loadPNG(image.name.c_str(), path);
					if (imageAsset.data) {
						VkExtent3D imagesize;
						imagesize.width = imageAsset.width;
						imagesize.height = imageAsset.height;
						imagesize.depth = 1;

						newImage = renderer->createImage(image.name.c_str(), imageAsset.data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);
					}
					destroyImageAsset(imageAsset);
				}
			},
			[&](fastgltf::sources::Vector& vector) {
				auto imageAsset = loadPNG(image.name.c_str(), vector.bytes.data(), vector.bytes.size());
				if (imageAsset.data) {
					VkExtent3D imagesize;
					imagesize.width = imageAsset.width;
					imagesize.height = imageAsset.height;
					imagesize.depth = 1;

					newImage = renderer->createImage(image.name.c_str(), imageAsset.data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);
				}
				destroyImageAsset(imageAsset);
			},
			[&](fastgltf::sources::BufferView& view) {
				auto& bufferView = asset.bufferViews[view.bufferViewIndex];
				auto& buffer = asset.buffers[bufferView.bufferIndex];

				std::visit(fastgltf::visitor{
										 [](auto& arg) {},
										 [&](fastgltf::sources::Vector& vector) {
											 if (view.mimeType == fastgltf::MimeType::KTX2) {
												 auto texture = createKTX2Image(image.name.c_str(), renderer->m_device, renderer->getCurrentFrame().m_commandPool, renderer->m_graphicsQueue, renderer->m_allocator, vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength, VK_FORMAT_BC7_SRGB_BLOCK, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
												 if (texture.has_value()) {
													 newImage = texture.value();
												 }
											 } else {
											 	 auto imageAsset = loadPNG(image.name.c_str(), vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength);
												 if (imageAsset.data) {
													 VkExtent3D imagesize;
													 imagesize.width = imageAsset.width;
													 imagesize.height = imageAsset.height;
													 imagesize.depth = 1;

													 newImage = renderer->createImage(image.name.c_str(), imageAsset.data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);
												 }
												 destroyImageAsset(imageAsset);
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

std::optional<Model> loadGLTF(VulkanRenderer* renderer, std::string_view filePath) {
	std::cout << std::format("Loading GLTF: {}", filePath) << '\n';
	if (!std::filesystem::exists(filePath)) {
		std::cout << std::format("Cannot load {}. File does not exist.\n", filePath);
		return {};
	}

	Model model{};

	fastgltf::Parser parser{ fastgltf::Extensions::KHR_texture_basisu };

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

	// Sampler loading
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
		samplerCreateInfo.maxAnisotropy = renderer->maxSamplerAnisotropy;
		samplerCreateInfo.anisotropyEnable = renderer->anisotropyEnabled;

		VkSampler newSampler{};
		vkCreateSampler(renderer->m_device, &samplerCreateInfo, nullptr, &newSampler);

		model.samplers.push_back(newSampler);
	}

	// Texture loading
	int defaultTextureCount = 0;
	auto textureStartTime = std::chrono::system_clock::now();
	for (fastgltf::Image& image : gltf.images) {
		auto img = loadImage(renderer, gltf, image);
		if (img.has_value()) {
			model.images.push_back(*img);
		} else {
			model.images.push_back(renderer->errorCheckerboardImage);
			defaultTextureCount++;
			std::cout << "gltf failed to load texture " << image.name << '\n';
		}
	}
	auto textureLoadTime = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - textureStartTime).count();
	std::cout << std::format("Loaded {} textures in {:.4f}ms. Errors: {}\n", model.images.size(), textureLoadTime, defaultTextureCount);


	// Material processing

	const auto materialOffset = MaterialCache_size(renderer->m_materialCache);
	auto sceneMaterialData = static_cast<MaterialData*>(renderer->globalMaterialDataBuffer.info.pMappedData);

	auto materialStartTime = std::chrono::system_clock::now();

	// NOTE: Add new materials starting from material offset
	uint32_t materialDataIndex = materialOffset;
	// NOTE: Add new textures starting from 1. 0 is default texture
	uint32_t bindlessTextureIndex = 1;
	for (fastgltf::Material& mat : gltf.materials) {
		Material newMat{};
		newMat.name = mat.name;

		newMat.materialData.colorFactors.x = mat.pbrData.baseColorFactor[0];
		newMat.materialData.colorFactors.y = mat.pbrData.baseColorFactor[1];
		newMat.materialData.colorFactors.z = mat.pbrData.baseColorFactor[2];
		newMat.materialData.colorFactors.w = mat.pbrData.baseColorFactor[3];

		newMat.materialData.metalRoughFactors.x = mat.pbrData.metallicFactor;
		newMat.materialData.metalRoughFactors.y = mat.pbrData.roughnessFactor;

		MaterialPass passType = MaterialPass::MainColor;
		if (mat.alphaMode == fastgltf::AlphaMode::Blend) {
			passType = MaterialPass::Transparent;
		} else if (mat.doubleSided) {
			passType = MaterialPass::DoubleSided;
		}

		// Set textures to "Default"
		newMat.materialData.albedoTexture = 0;
		newMat.materialData.normalTexture = 0;
		newMat.materialData.specularTexture = 0;
		newMat.materialData.emissiveTexture = 0;

		// grab textures from gltf file
		// TODO: Set rest of the textures
		if (mat.pbrData.baseColorTexture.has_value()) {
			auto textureIndex = mat.pbrData.baseColorTexture.value().textureIndex;
			auto texture = gltf.textures[textureIndex];
			size_t img{};

			if (texture.basisuImageIndex.has_value()) {
				img = gltf.textures[textureIndex].basisuImageIndex.value();
			} else {
				img = gltf.textures[textureIndex].imageIndex.value();
			}
			size_t sampler = gltf.textures[textureIndex].samplerIndex.value();
			newMat.materialData.albedoTexture = bindlessTextureIndex;

			auto imageSampler = model.images[img].sampler ? model.images[img].sampler : model.samplers[sampler];

			// TODO: We are writing textures 1 by 1. We should batch these.
			renderer->writeBindlessTextureToGlobalDescriptor(renderer->bindlessTexturesDescriptorSet, model.images[img], imageSampler, bindlessTextureIndex);
			bindlessTextureIndex++;
		}

		// write material data to buffer
		sceneMaterialData[materialDataIndex] = newMat.materialData;

		// build material
		newMat.passType = passType;

		if (passType == MaterialPass::Transparent) {
			newMat.pipeline = &renderer->transparentPipeline;
		} else if (passType == MaterialPass::DoubleSided) {
			newMat.pipeline = &renderer->doubleSidedPipeline;
		} else {
			newMat.pipeline = &renderer->opaquePipeline;
		}

		model.materials.push_back(newMat);

		// Add new material to our global material cache. If we add it, increase index.
		// TODO: handle indices for duplicate materials.
		if (MaterialCache_add(renderer->m_materialCache, materialDataIndex, newMat)) {
			materialDataIndex++;
		}
	}

	auto materialLoadTime = std::chrono::duration<double, std::micro>(std::chrono::system_clock::now() - materialStartTime).count();
	std::cout << std::format("Loaded {} materials in {:.4f}us\n", model.materials.size(), materialLoadTime);

	// Geometry processing
	std::vector<Mesh> meshes{};
	meshes.reserve(gltf.meshes.size());

	auto geometryProcessingStart = std::chrono::system_clock::now();

	std::vector<uint32_t> indices;
	std::vector<Vertex> vertices;

	for (auto& mesh : gltf.meshes) {
		Mesh newmesh{};
		newmesh.name = mesh.name;

		for (auto&& p : mesh.primitives) {
			MeshPrimitive newPrimitive{};
			auto vertexOffset = static_cast<int32_t>(vertices.size());

			newPrimitive.firstIndex = (uint32_t)indices.size();
			newPrimitive.indexCount = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;
			newPrimitive.vertexOffset = vertexOffset;

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

			// load tangents
			auto tangents = p.findAttribute("TANGENT");
			if (tangents != p.attributes.end()) {

				fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*tangents).second], [&](glm::vec4 t, size_t index) {
					vertices[vertexOffset + index].tangent = t;
				});
			}


			if (p.materialIndex.has_value()) {
				newPrimitive.materialIndex = p.materialIndex.value() + 1;
			} else {
				newPrimitive.materialIndex = 0;// set default material
			}

			auto mat = MaterialCache_get(renderer->m_materialCache, newPrimitive.materialIndex);
			newPrimitive.passType = mat.passType;

			newmesh.primitives.push_back(newPrimitive);
		}

		meshes.push_back(newmesh);
	}
	auto geometryProcessingTime = std::chrono::duration<double>(std::chrono::system_clock::now() - geometryProcessingStart).count();

	auto geometryUploadStart = std::chrono::system_clock::now();
	model.modelBuffers = renderer->uploadMesh<Vertex>(indices, vertices, "modelBuffers");
	auto geometryUploadTime = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - geometryUploadStart).count();
	std::cout << std::format("Loaded {} meshes in {:.4f}s. Uploaded buffers in {:.4f}ms\n", meshes.size(), geometryProcessingTime, geometryUploadTime);

	std::vector<Entity*> entities;

	// Entity processing
	auto entityStartTime = std::chrono::system_clock::now();
	// load all nodes and their meshes
	for (fastgltf::Node& node : gltf.nodes) {
		auto entity = new Entity();

		// find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the meshnode class
		if (node.meshIndex.has_value()) {
			entity->mesh = std::make_shared<Mesh>(meshes[*node.meshIndex]);
		} else {
			entity->mesh = nullptr;
		}

		entities.push_back(entity);

		std::visit(fastgltf::visitor{
								 [&](fastgltf::Node::TransformMatrix matrix) {
									 memcpy(&entity->localTransform, matrix.data(), sizeof(matrix));
								 },
								 [&](fastgltf::Node::TRS transform) {
									 glm::vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
									 glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);
									 glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

									 glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
									 glm::mat4 rm = glm::toMat4(rot);
									 glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

									 entity->localTransform = tm * rm * sm;
								 } },
			node.transform);
	}

	// Setup transform hierarchy
	for (int i = 0; i < gltf.nodes.size(); i++) {
		fastgltf::Node& node = gltf.nodes[i];
		auto& entity = entities[i];

		for (auto& c : node.children) {
			entity->children.push_back(entities[c]);
			entities[c]->parent = entity;
		}
	}

	// find the top nodes, with no parents
	std::vector<Entity*> topEntities{};
	for (auto& entity : entities) {
		if (entity->parent == nullptr) {
			topEntities.push_back(entity);
		}
	}

	// If we only have one root, we are done
	if (topEntities.size() == 1) {
		model.root = std::move(topEntities[0]);
	} else if(topEntities.size() > 1) {
		// Create a new root entity to include all the "top nodes" from GLTF scene
		auto rootEntity = new Entity();
		rootEntity->localTransform = glm::mat4{ 1.0f };
		rootEntity->children.insert(rootEntity->children.begin(), topEntities.begin(), topEntities.end());
		rootEntity->mesh = nullptr;
		rootEntity->materialIndex = 0;
		model.root = std::move(rootEntity);
	} else {
		// Error? We have no top nodes?
		std::cout << "No top nodes?\n";
	}

	auto entitiesLoadTime = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - entityStartTime).count();
	std::cout << std::format("Loaded {} Entities in {:.4f}ms\n", entities.size(), entitiesLoadTime);


	return model;
}

void cleanupModel(VulkanRenderer* renderer, Model& model) {
	VkDevice dv = renderer->m_device;

	renderer->destroyBuffer(model.modelBuffers.indexBuffer);
	renderer->destroyBuffer(model.modelBuffers.vertexBuffer);

	cleanupModelEntities(model.root);

	for (auto& image : model.images) {
		if (image.image == renderer->errorCheckerboardImage.image) {
			// dont destroy the default images
			continue;
		}
		renderer->destroyImage(image);
	}

	for (auto& sampler : model.samplers) {
		vkDestroySampler(dv, sampler, nullptr);
	}

	// TODO: should be handled by the renderer
	renderer->destroyBuffer(renderer->globalMaterialDataBuffer);
}

// Flatten the hierarchy and delete all entities
void cleanupModelEntities(Entity* root) {
	std::vector<Entity*> flatEntities{};
	Entity_flattenHierarchyNoTransform(root, flatEntities);

	for (auto& entity : flatEntities) {
		delete entity;
	}
}

}// namespace pm
