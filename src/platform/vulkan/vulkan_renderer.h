#pragma once

#include "material.h"
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
	VkDeviceAddress vertexBufferAddress;
};


// NOTE: keeping these separate because we might want to include extra stuff here later on.
struct UIIndirectCommand {
	uint32_t drawId;
	VkDrawIndexedIndirectCommand command;
};

struct alignas(16) MeshDraw {
	glm::mat4 transform{};
	uint32_t materialIndex{};
	float padding[3]{ 0.0f, 0.0f, 0.0f };
};

struct MeshIndirectCommand {
	uint32_t drawId;
	VkDrawIndexedIndirectCommand command;
};

struct DrawBatchDescriptor {
	int32_t binding;
	AllocatedBuffer buffer;
	uint32_t size;
	uint32_t offset;
	VkDescriptorType type;
};

struct DrawBatchCommands {
	AllocatedBuffer buffer;
	uint32_t offset;
	uint32_t size;
	uint32_t stride;
};

struct DrawBatch {
	DrawBatchCommands commands{};
	std::vector<DrawBatchDescriptor> descriptors{};
	GPUMeshBuffers* meshBuffers{};
	VkPipeline pipeline{};
	VkPipelineLayout pipelineLayout{};
	VkDescriptorSetLayout descriptorSetLayout{};
};

struct UIUniformData {
	glm::mat4 projection;
	glm::mat4 view;
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

constexpr uint32_t FRAME_OVERLAP = 2;

struct RendererStats {
	double frametime{};
	double frameGpuTimeAvg{};
	double uiFrametimeAvg{};
	double sceneUpdateTimeAvg{};
	double meshDrawTimeAvg{};
	double drawBatchGenerationTimeAvg{};

	uint32_t triangleCount{};
	uint32_t drawCallCount{};
};

struct VulkanRendererConfig {
	bool useValidationLayers;
	VkExtent2D windowExtent;
	SDL_Window* window;
	std::shared_ptr<Camera> mainCamera;
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

	std::vector<DrawBatch> drawBatches{};
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

struct RenderObject {
	uint32_t drawId;
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t indexCount;
	glm::mat4 transform;
	uint32_t materialIndex;
};

struct ModelDrawRender {
	Material* material;

	std::vector<RenderObject> renderObjects{};
};

struct DrawContext {
	ModelDrawRender opaqueDraws{};
	ModelDrawRender transparentDraws{};

	GPUMeshBuffers* modelBuffers;
	uint32_t nodeCount = 0;
};

class VulkanRenderer {
public:
	void init(VulkanRendererConfig* state);

	// NOTE: load some default data for our engine to draw
	void initDefaultData();

	void buildDrawBatches(std::vector<Model*>& models);

	// drawing
	void draw(float deltaTime);
	void drawBackground(VkCommandBuffer commandBuffer);
	void drawGeometry(VkCommandBuffer commandBuffer);

	void cleanup();

	// Buffers
	AllocatedBuffer createBuffer(std::string name, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void destroyBuffer(const AllocatedBuffer& buffer);

	// Images
	AllocatedImage createImage(std::string name, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	AllocatedImage createImage(std::string name, void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	void destroyImage(const AllocatedImage& img);

	template<typename VertexType>
	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<VertexType> vertices, std::string name);

	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	void resizeSwapchain();

	VkDevice m_device;
	VkDescriptorSetLayout m_gpuSceneDataDescriptorLayout;
	VkDescriptorSetLayout m_modelDrawDescriptorLayout;
	AllocatedImage m_drawImage;
	AllocatedImage m_depthImage;

	DeletionQueue m_mainDeletionQueue;

	DrawContext mainDrawContext;

	// Font Rendering
	void generateText(std::string stats, std::string fps);
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

	std::unordered_map<std::string, Model> loadedModels;

	// bindless textures
	VkDescriptorPool bindlessPool;
	VkDescriptorSetLayout bindlessTexturesSetLayout;
	VkDescriptorSet bindlessTexturesDescriptorSet;

	VkDescriptorSet materialsDescriptor;
	AllocatedBuffer globalMaterialDataBuffer;

	VkPhysicalDevice m_chosenGPU;
	FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; };
	VkQueue m_graphicsQueue{};

	// KTX2 formats
	std::vector<ktx_transcode_fmt_e> availableTargetFormats{};
	std::vector<std::string> availableTargetFormatsNames{};

	// Allocator
	VmaAllocator m_allocator;

	bool anisotropyEnabled;
	float maxSamplerAnisotropy;

	void writeBindlessTextureToGlobalDescriptor(VkDescriptorSet bindlessTextureSet, AllocatedImage& image, VkSampler sampler, uint32_t index);

	// Default pipelines
	void buildDefaultPipelines();
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;
	MaterialPipeline doubleSidedPipeline;

	MaterialCache m_materialCache;

private:
	void initVulkan();
	void initSwapchain();
	void initCommands();
	void initSyncStructures();
	void initDescriptors();
	void initPipelines();
	void initFontData();
	void initUI();
	void initBindlessTextureDescriptor();
	void initQueryPools();

	// specific pipelines
	void initBackgroundPipelines();
	void initFontPipeline();
	void initUIPipeline();

	VulkanRendererConfig* m_rendererState;
	float m_renderScale{ 1.0f };

	// Structures for immediateSubmit
	VkFence m_immFence;
	VkCommandBuffer m_immCommandBuffer;
	VkCommandPool m_immCommandPool;

	// Vulkan init stuff
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debug_messenger;
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
	uint32_t m_graphicsQueueFamily{};

	// Draw resources
	VkExtent2D m_drawExtent;

	// Descriptors
	DescriptorAllocator m_globalDescriptorAllocator;
	VkDescriptorSet m_drawImageDescriptors;
	VkDescriptorSetLayout m_drawImageDescriptorLayout;

	// Global scene data for all meshes
	GPUSceneData m_sceneData;
	
	// Compute pipeline
	VkPipeline m_skyPipeline;
	VkPipelineLayout m_skyPipelineLayout;

	// Text rendering
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
	std::vector<UIVertex> fontVertices;
	std::vector<uint32_t> fontIndices;
	std::vector<UIElement> textElements{};

	// UI Rendering
	UIUniformData uiUniformData{};
	AllocatedBuffer uiUniformBuffer;
	DescriptorAllocator uiDescriptorAllocator;
	VkDescriptorSetLayout uiDescriptorLayout;
	VkDescriptorSet uiDescriptorSet;
	VkPipelineLayout uiPipelineLayout;
	VkPipeline uiPipeline;
	std::vector<UIVertex> uiVertices;
	std::vector<uint32_t> uiIndices;
	std::vector<UIElement> uiElements{};

	// Timestamp
	float physicalDeviceTimestampPeriod{};
	VkQueryPool timestampPool;
	VkQueryPool pipelineStatisticsPool;
};

}// namespace pm
