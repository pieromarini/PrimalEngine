#pragma once

#include "assets/asset.h"
#include "material.h"
#include "platform/vulkan/swapchain.h"
#include "vulkan_loader.h"
#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <string_view>
#include <unordered_map>
#include <vulkan/vulkan.h>

#include <ranges>

#include "camera.h"
#include "utils/fonts.h"
#include "vk_types.h"
#include "vulkan_descriptor.h"
#include "vulkan_texture.h"

#include "ui/ui_types.h"
#include "ui/ui_manager.h"
#include "platform/window.h"

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
	VkDeviceAddress vertexBuffer;
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

struct alignas(16) UIDrawData {
	glm::mat4 transform{};
	uint32_t materialIndex{};
	float padding[3]{ 0.0f, 0.0f, 0.0f };
};

struct alignas(16) ViewportDrawData {
	glm::mat4 transform{};
	uint32_t textureIndex{};
};

struct alignas(16) UIMaterialData {
	glm::vec4 backgroundColor;
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
struct DrawBatchImageDescriptor {
	int32_t binding;
	// TODO: replace with AllocatedImage when refactoring the SDF loading code.
	VkImageView imageView;
	VkSampler sampler;
	VkImageLayout imageLayout;
	VkDescriptorType type;
};

struct DrawBatchCommands {
	AllocatedBuffer buffer;
	uint32_t offset;
	uint32_t size;
	uint32_t stride;
};

enum DrawBatchType {
	MESH_BATCH,
	UI_BATCH,
	TEXT_BATCH,
	VIEWPORT_BATCH
};

struct DrawBatch {
	DrawBatchType type; // TODO(piero): Remove this. This is only used to bind global descriptor sets for a batch but we should include global descriptor sets in the batch itself.
	DrawBatchCommands commands{};
	std::vector<DrawBatchDescriptor> descriptors{};
	std::vector<DrawBatchImageDescriptor> imageDescriptors{};
	GPUMeshBuffers meshBuffers{};
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

	// 3D draw batch generation
	double entityFlattenTimeAvg{};
	double drawBatchGenerationTimeAvg{};

	// UI draw batch generation
	double uiDrawBatchGenerationTimeAvg{};
	double uiLayoutTimeAvg{};

	uint32_t triangleCount{};
	uint32_t drawCallCount{};
};

struct VulkanRendererConfig {
	PrimalWindow* window;
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
	std::vector<DrawBatch> uiDrawBatches{};
	FixedArray<UI::UIRenderCommand> uiRenderCommands{};
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

class VulkanRenderer {
public:
	void init();
	void setup();
	void setInitialState(VulkanRendererConfig* state);

	void loadTestScene();

	void initDefaultData();

	void buildDrawBatches(std::vector<Model*>& models);
	void buildUIDrawBatches(FixedArray<UI::UIRenderCommand>& renderCommands);
	std::vector<UI::UIElement> buildUIGeometry(FixedArray<UI::UIRenderCommand>& renderCommands, std::vector<UI::UIVertex>& vertices, std::vector<uint32_t>& indices);

	// drawing
	void draw();
	void drawBackground(VkCommandBuffer commandBuffer);
	void drawGeometry(VkCommandBuffer commandBuffer);
	void drawUI(VkCommandBuffer commandBuffer);

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
	void resizeSwapchain(PrimalWindow* window);

	VkDescriptorSetLayout m_gpuSceneDataDescriptorLayout;
	VkDescriptorSetLayout m_modelDrawDescriptorLayout;
	AllocatedImage m_drawImage;
	AllocatedImage m_depthImage;

	// NOTE(piero): Testing viewport rendering
	AllocatedImage m_sceneDrawImage;
	AllocatedImage m_sceneDepthImage;
	VkDescriptorSet m_sceneDrawImageDescriptor;
	VkDescriptorSetLayout m_sceneDrawImageDescriptorLayout;

	VkDescriptorSetLayout viewportDescriptorLayout;
	VkDescriptorSet viewportDescriptorSet;
	VkPipelineLayout viewportPipelineLayout;
	VkPipeline viewportPipeline;

	DeletionQueue m_mainDeletionQueue;

	// Font Rendering
	void updateScene(float deltaTime);
	void updateFontData();
	void updateUIData();

	void update(float deltaTime);

	void setPointerState(float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);

	// Image testing
	AllocatedImage whiteImage;
	AllocatedImage blackImage;
	AllocatedImage greyImage;
	AllocatedImage errorCheckerboardImage;

	VkSampler defaultSamplerLinear;
	VkSampler defaultSamplerNearest;

	std::unordered_map<std::string, Model> loadedModels;

	// Bindless Global texture arrays

	// Mesh textures
	VkDescriptorPool bindlessPool;
	VkDescriptorSetLayout bindlessTexturesSetLayout;
	VkDescriptorSet bindlessTexturesDescriptorSet;

	// Viewport textures
	VkDescriptorPool viewportDescriptorPool;
	VkDescriptorSetLayout viewportTextureSetLayout;
	VkDescriptorSet viewportTextureDescriptorSet;

	// store registered texture id from UI system to render into viewport
	uint32_t sceneTextureId;

	// Global materials
	VkDescriptorSet materialsDescriptor;
	AllocatedBuffer globalMaterialDataBuffer;

	VkDevice m_device;
	VkPhysicalDevice m_chosenGPU;
	VkInstance m_instance;


	uint32_t getCurrentFrameIndex() { return m_frameNumber % FRAME_OVERLAP; }
	FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; };
	VkQueue m_graphicsQueue{};

	// KTX2 formats
	std::vector<ktx_transcode_fmt_e> availableTargetFormats{};
	std::vector<std::string> availableTargetFormatsNames{};

	// Allocator
	VmaAllocator m_allocator;

	bool anisotropyEnabled;
	float maxSamplerAnisotropy;

	void writeBindlessTextureToGlobalDescriptor(VkDescriptorSet bindlessTextureSet, uint32_t binding, AllocatedImage& image, VkSampler sampler, uint32_t index);

	// Default pipelines
	void buildDefaultPipelines();
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;
	MaterialPipeline doubleSidedPipeline;

	MaterialCache m_materialCache;

	// image registering for viewport rendering
	uint32_t registerImage(AllocatedImage* image);
	std::vector<AllocatedImage*> registeredImages;

private:
	void initVulkan();
	void initRenderTargets();
	void initCommands();
	void initSyncStructures();
	void initDescriptors();
	void initPipelines();
	void initFontData();
	void initUI();
	void initBindlessTextureDescriptor(VkDescriptorPool& pool, VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet& descriptotSet);
	void initQueryPools();

	// specific pipelines
	void initBackgroundPipelines();
	void initFontPipeline();
	void initViewportPipeline();
	void initUIPipeline();

	VulkanRendererConfig* m_rendererState;
	float m_renderScale{ 1.0f };

	// Structures for immediateSubmit
	VkFence m_immFence;
	VkCommandBuffer m_immCommandBuffer;
	VkCommandPool m_immCommandPool;

	// Vulkan init stuff
	VkDebugUtilsMessengerEXT m_debug_messenger;
	VkSurfaceKHR m_surface;

	// Swapchain
	PrimalSwapchain mainSwapchain;

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
	FontUniformData fontUniformData{};
	AllocatedBuffer fontUniformBuffer;
	DescriptorAllocator fontDescriptorAllocator;
	VkDescriptorSetLayout fontDescriptorLayout;
	VkDescriptorSet fontDescriptorSet;
	uint32_t fontIndexCount{ 0 };
	VkPipelineLayout fontPipelineLayout;
	VkPipeline fontPipeline;

	// UI Rendering
	UIUniformData uiUniformData{};
	AllocatedBuffer uiUniformBuffer;
	DescriptorAllocator uiDescriptorAllocator;
	VkDescriptorSetLayout uiDescriptorLayout;
	VkDescriptorSet uiDescriptorSet;
	VkPipelineLayout uiPipelineLayout;
	VkPipeline uiPipeline;

	VkPipelineCache m_pipelineCache;

	// Timestamp
	float physicalDeviceTimestampPeriod{};
	VkQueryPool timestampPool;
	VkQueryPool pipelineStatisticsPool;

	// Testing fonts
	FontAsset arialFont;
	FontAsset sourceCodeFont;
	AllocatedImage sourceCodeFontTexture;

	// Memory Arenas
	MemoryArena uiMemoryArena;
};

}// namespace pm
