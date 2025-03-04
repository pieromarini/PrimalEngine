#pragma once

#include "vulkan_loader.h"
#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include <ranges>

#include "camera.h"
#include "utils/fonts.h"
#include "vk_types.h"
#include "vulkan_descriptor.h"
#include "vulkan_texture.h"

namespace pm {

struct FontUniformData {
	// Scene matrices
	glm::mat4 projection;
	glm::mat4 view;

	// Font display options
	glm::vec4 outlineColor{ 1.0f, 0.0f, 0.0f, 0.0f };
	float outlineWidth{ 0.6f };
	float outline{ true };
};

struct UIPushConstants {
	glm::mat4 transform;
};

struct UIUniformData {
	glm::mat4 projection;
	glm::mat4 view;
};
struct UIVertex {
	glm::vec2 position;
	glm::vec3 color;
	glm::vec2 uv;
};


class DeletionQueue {
public:
	void push(std::function<void()>&& function) {
		deletors.push_back(std::move(function));
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto& deletor : std::ranges::reverse_view(deletors)) {
			deletor();
		}

		deletors.clear();
	}

private:
	std::deque<std::function<void()>> deletors{};
};

struct RendererStats {
	float frametime;
	float uiFrametime;
	int triangleCount;
	int drawCallCount;
	float sceneUpdateTime;
	float meshDrawTime;
};

struct VulkanRendererConfig {
	bool useValidationLayers;
	VkExtent2D windowExtent;
	SDL_Window* window;
	Camera* mainCamera;
	bool resizeRequested;
	RendererStats rendererStats;
};

struct FrameData {
	VkCommandPool m_commandPool;
	VkCommandBuffer m_commandBuffer;

	VkSemaphore m_swapchainSemaphore;
	VkSemaphore m_renderSemaphore;

	VkFence m_renderFence;

	DeletionQueue m_deletionQueue;

	DescriptorAllocator m_frameDescriptors;
};

struct AllocatedImage {
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

struct GPUSceneData {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection;
	glm::vec4 sunlightColor;
};

struct ComputePushConstants {
	glm::vec4 data1;
	glm::vec4 data2;
	glm::vec4 data3;
	glm::vec4 data4;
};

struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;
	MaterialPipeline doubleSidedPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors;
		glm::vec4 metalRoughFactors;
		// padding, we need it anyway for uniform buffers
		glm::vec4 padding[14];
	};

	struct MaterialResources {
		AllocatedImage colorImage;
		VkSampler colorSampler;
		AllocatedImage metalRoughImage;
		VkSampler metalRoughSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	DescriptorWriter writer;

	void buildPipelines(VulkanRenderer* renderer);
	void clearResources(VkDevice device);

	MaterialInstance writeMaterial(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocator& descriptorAllocator);
};

struct MeshNode : public Node {
	std::shared_ptr<MeshAsset> mesh;

	void draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
};

struct RenderObject {
	uint32_t indexCount;
	uint32_t firstIndex;
	VkBuffer indexBuffer;

	MaterialInstance* material;

	glm::mat4 transform;
	VkDeviceAddress vertexBufferAddress;
};

struct DrawContext {
	std::vector<RenderObject> opaqueSurfaces;
	std::vector<RenderObject> transparentSurfaces;
};

constexpr uint32_t FRAME_OVERLAP = 2;

class VulkanRenderer {
public:
	void init(VulkanRendererConfig* state);

	// NOTE: load some default data for our engine to draw
	void initDefaultData();

	// drawing
	void draw(float deltaTime);
	void drawBackground(VkCommandBuffer commandBuffer);
	void drawGeometry(VkCommandBuffer commandBuffer);

	void cleanup();

	// Buffers
	AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void destroyBuffer(const AllocatedBuffer& buffer);

	// Images
	AllocatedImage createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	AllocatedImage createImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	void destroyImage(const AllocatedImage& img);

	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	void resizeSwapchain();

	VkDevice m_device;
	VkDescriptorSetLayout m_gpuSceneDataDescriptorLayout;
	AllocatedImage m_drawImage;
	AllocatedImage m_depthImage;

	DeletionQueue m_mainDeletionQueue;

	DrawContext mainDrawContext;
	std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;

	// Font Rendering
	void generateText(std::string text);
	void updateScene(float deltaTime);
	void updateFontData();
	void updateUIData();

	// Image testing
	AllocatedImage whiteImage;
	AllocatedImage blackImage;
	AllocatedImage greyImage;
	AllocatedImage errorCheckerboardImage;

	VkSampler defaultSamplerLinear;
	VkSampler defaultSamplerNearest;

	// GLTF loading testing
	MaterialInstance defaultData;
	GLTFMetallic_Roughness metalRoughMaterial;

	std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;

private:
	void initVulkan();
	void initSwapchain();
	void initCommands();
	void initSyncStructures();
	void initDescriptors();
	void initPipelines();
	void initFontData();
	void initUI();

	// specific pipelines
	void initBackgroundPipelines();
	void initFontPipeline();
	void initUIPipeline();
	void initMeshPipeline();

	VulkanRendererConfig* m_rendererState;
	float m_renderScale{ 1.0f };

	// Structures for immediateSubmit
	VkFence m_immFence;
	VkCommandBuffer m_immCommandBuffer;
	VkCommandPool m_immCommandPool;

	// Vulkan init stuff
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debug_messenger;
	VkPhysicalDevice m_chosenGPU;
	VkSurfaceKHR m_surface;

	// Swapchain
	void createSwapchain(uint32_t width, uint32_t height);
	void destroySwapchain();
	VkSwapchainKHR m_swapchain;
	VkFormat m_swapchainImageFormat;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	VkExtent2D m_swapchainExtent;

	// Commands
	FrameData m_frames[FRAME_OVERLAP]{};
	uint32_t m_frameNumber{};
	FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; };
	VkQueue m_graphicsQueue{};
	uint32_t m_graphicsQueueFamily{};

	// Allocator
	VmaAllocator m_allocator;

	// Draw resources
	VkExtent2D m_drawExtent;

	// Descriptors
	DescriptorAllocator m_globalDescriptorAllocator;
	VkDescriptorSet m_drawImageDescriptors;
	VkDescriptorSetLayout m_drawImageDescriptorLayout;

	// Scene data tied to DescriptorSetLayout
	GPUSceneData m_sceneData;

	// Compute pipeline
	VkPipeline m_skyPipeline;
	VkPipelineLayout m_skyPipelineLayout;

	// Loaded meshes from GLTF file
	GPUMeshBuffers rectangle;
	std::vector<std::shared_ptr<MeshAsset>> m_testMeshes;

	// Text rendering
	VkCommandPool m_fontCommandPool;// TEMP
	VkCommandBuffer fontCommandBuffer;
	Texture2D fontSDF;
	FontUniformData fontUniformData{};
	AllocatedBuffer fontUniformBuffer;
	DescriptorAllocator fontDescriptorAllocator;
	VkDescriptorSetLayout fontDescriptorLayout;
	VkDescriptorSet fontDescriptorSet;
	std::array<bmchar, 255> fontChars;
	uint32_t fontIndexCount{ 0 };
	VkPipelineLayout fontPipelineLayout;
	VkPipeline fontPipeline;
	AllocatedBuffer textVertexBuffer;
	AllocatedBuffer textIndexBuffer;

	// UI Rendering
	UIUniformData uiUniformData{};
	AllocatedBuffer uiUniformBuffer;
	DescriptorAllocator uiDescriptorAllocator;
	VkDescriptorSetLayout uiDescriptorLayout;
	VkDescriptorSet uiDescriptorSet;
	VkPipelineLayout uiPipelineLayout;
	VkPipeline uiPipeline;
	AllocatedBuffer uiVertexBuffer;
	AllocatedBuffer uiIndexBuffer;
};

}// namespace pm
