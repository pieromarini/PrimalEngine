#pragma once

#include "assets/asset.h"
#include "material.h"
#include "terrain/voxel.h"
#include "vulkan_loader.h"
#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

#include <ranges>

#include "camera.h"
#include "vk_types.h"
#include "vulkan_descriptor.h"
#include "vulkan_texture.h"

#include "platform/window.h"
#include "ui/ui_types.h"
#include "memory/arena.h"


namespace pm {

struct UIPushConstants {
	VkDeviceAddress vertexBuffer;
	float screenWidth;
	float screenHeight;
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
	float horizontalBorder{ 0.0f };
	float verticalBorder{ 0.0f };
	float padding[2];
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
	DrawBatchType type;// TODO(piero): Remove this. This is only used to bind global descriptor sets for a batch but we should include global descriptor sets in the batch itself.
	DrawBatchCommands commands{};
	std::vector<DrawBatchDescriptor> descriptors{};
	std::vector<DrawBatchImageDescriptor> imageDescriptors{};
	GPUMeshBuffers meshBuffers{};
	VkPipeline pipeline{};
	VkPipelineLayout pipelineLayout{};
	VkDescriptorSetLayout descriptorSetLayout{};
};

// Group batches for a specific window
struct UIWindowBatch {
	PrimalWindow* window;
	std::vector<DrawBatch> drawBatches;
};

struct GBuffer {
	AllocatedImage albedo;
	AllocatedImage irradiance;
	AllocatedImage depth;

	AllocatedBuffer gridInfo;
	AllocatedBuffer voxelData;
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

	uint32_t frameCount{ 0 };
};

struct VulkanRendererConfig {
	PrimalWindow* window{};
	Camera* mainCamera{};
	bool resizeRequested{};
	RendererStats rendererStats;
};

struct FrameData {
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	VkFence renderFence;

	DeletionQueue deletionQueue;

	DescriptorAllocator frameDescriptor;

	std::vector<DrawBatch> drawBatches{};
	std::vector<UIWindowBatch> uiWindowBatches{};

	MemoryArena perFrameArena; // TODO(piero): use arena for per-frame allocations
	FixedArray<UI::UIWindowBatchCommands> uiWindowBatchCommands{};
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
	glm::vec4 viewPosition;
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

struct VulkanRendererContext {
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkInstance instance;
	VkSurfaceKHR surface;

	VkQueue graphicsQueue{};
	VkDebugUtilsMessengerEXT debugMessenger{};

	// KTX2 formats
	std::vector<ktx_transcode_fmt_e> availableTargetFormats{};
	std::vector<std::string> availableTargetFormatsNames{};

	// VMA
	VmaAllocator vmaAllocator;

	// scene
	std::vector<Model> loadedModels;

	// Per-frame data
	FrameData frames[FRAME_OVERLAP]{};
	uint32_t frameNumber{};
	uint32_t graphicsQueueFamily{};

	// Deletion queues
	DeletionQueue mainDeletionQueue{};

	// Descriptor allocators
	DescriptorAllocator globalDescriptorAllocator{};

	// Descriptor set Layouts
	VkDescriptorSetLayout gpuSceneDataDescriptorLayout{};
	VkDescriptorSetLayout modelDrawDescriptorLayout{};
	VkDescriptorSetLayout drawImageDescriptorLayout{};
	VkDescriptorSetLayout viewportDescriptorLayout{};
	VkDescriptorSetLayout fontDescriptorLayout{};
	VkDescriptorSetLayout uiDescriptorLayout{};

	// Descriptor sets
	VkDescriptorSet drawImageDescriptors{};

	// BINDLESS DESCRIPTORS
	// Mesh textures
	VkDescriptorPool bindlessPool;
	VkDescriptorSetLayout bindlessTexturesSetLayout{};
	VkDescriptorSet bindlessTexturesDescriptorSet{};

	// Viewport textures
	VkDescriptorPool viewportDescriptorPool{};
	VkDescriptorSetLayout viewportTextureSetLayout{};
	VkDescriptorSet viewportTextureDescriptorSet{};

	// Viewport rendering
	VkPipelineLayout viewportPipelineLayout{};
	VkPipeline viewportPipeline{};

	// Pipelines
	VkPipeline fontPipeline{};
	VkPipelineLayout fontPipelineLayout{};
	VkPipeline uiPipeline{};
	VkPipelineLayout uiPipelineLayout{};

	VkPipelineCache pipelineCache{};

	// Images
	AllocatedImage sceneDrawImage{};// viewport
	AllocatedImage sceneDepthImage{};// viewport
	uint32_t sceneTextureId{};

	// Buffers
	AllocatedBuffer globalMaterialDataBuffer{};

	// Structures for immediateSubmit
	VkFence immFence{};
	VkCommandBuffer immCommandBuffer{};
	VkCommandPool immCommandPool{};

	// Default data
	AllocatedImage whiteImage{};
	AllocatedImage blackImage{};
	AllocatedImage greyImage{};
	AllocatedImage errorCheckerboardImage{};
	VkSampler defaultSamplerLinear{};
	VkSampler defaultSamplerNearest{};

	// Global scene data for all meshes
	GPUSceneData sceneData{};

	// Testing fonts
	FontAsset arialFont{};
	FontAsset sourceCodeFont{};
	AllocatedImage sourceCodeFontTexture{};

	// Default 3d pipelines
	MaterialPipeline opaquePipeline{};
	MaterialPipeline transparentPipeline{};
	MaterialPipeline doubleSidedPipeline{};

	// Storage
	MaterialCache materialCache{};
	std::vector<AllocatedImage*> registeredImages{};

	// config
	bool anisotropyEnabled{};
	float maxSamplerAnisotropy{};

	VulkanRendererConfig* rendererState{};
	float renderScale{ 1.0f };

	// Timestamp
	float physicalDeviceTimestampPeriod{};
	VkQueryPool timestampPool{};
	VkQueryPool pipelineStatisticsPool{};

	bool testBool;

	// Memory
	MemoryArena uiMemoryArena;

	// Terrain test
	VoxelTerrain voxelTerrain;
	MaterialPipeline voxelPipeline;
	VkDescriptorSetLayout voxelDescriptorLayout;
	GPUMeshBuffers voxelMeshBuffers;
	AllocatedBuffer voxelDrawCommandsBuffer;
	uint32_t voxelDrawCommandsCount;

	// GBuffer
	GBuffer gbuffer;
	VkDescriptorSet gbufferDescriptorSet;
	VkDescriptorSetLayout gbufferDescriptorLayout;
	VkPipelineLayout gbufferPipelineLayout;
	VkPipeline gbufferPipeline;

	// Write GBuffer to render target using a compute shader
	VkDescriptorSet resolveDescriptorSet;
	VkDescriptorSetLayout resolveDescriptorLayout;
	VkPipelineLayout resolvePipelineLayout;
	VkPipeline resolvePipeline;

	AllocatedImage blueNoise;

	uint32_t gbufferDebugChannel{ 0 };

	// TODO(piero): Should refactor this. Used to render a different layout to test full screen viewport rendering
	bool fullScreen{ false };
};

inline uint32_t getCurrentFrameIndex(VulkanRendererContext* context) {
	return context->frameNumber % FRAME_OVERLAP;
}

inline FrameData& getCurrentFrame(VulkanRendererContext* context) {
	return context->frames[context->frameNumber % FRAME_OVERLAP];
};

void rendererInit(VulkanRendererContext* context);
void rendererSetup(VulkanRendererContext* context);
void rendererSetInitialState(VulkanRendererContext* context, VulkanRendererConfig* state);

void rendererInitMemory(VulkanRendererContext* context);

void loadTestScene(VulkanRendererContext* context);
void terrainTest(VulkanRendererContext* context);

void rendererInitDefaultData(VulkanRendererContext* context);

void initVulkan(VulkanRendererContext* context);
void initRenderTargets(VulkanRendererContext* context);
void initCommands(VulkanRendererContext* context);
void initSyncStructures(VulkanRendererContext* context);
void initDescriptors(VulkanRendererContext* context);
void initPipelines(VulkanRendererContext* context);
void initQueryPools(VulkanRendererContext* context);

void initFontData(VulkanRendererContext* context);
void initUI(VulkanRendererContext* context);
void initBindlessTextureDescriptor(VulkanRendererContext* context, VkDescriptorPool& pool, VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet& descriptotSet);

void resizeSwapchain(VulkanRendererContext* context, PrimalWindow* window);
void resizeRenderTargets(VulkanRendererContext* context);

// specific pipelines
// TODO(piero): Init pipelines from config file
void initFontPipeline(VulkanRendererContext* context);
void initViewportPipeline(VulkanRendererContext* context);
void initUIPipeline(VulkanRendererContext* context);
void buildDefaultPipelines(VulkanRendererContext* context);

void initVoxelPipeline(VulkanRendererContext* context);

void writeBindlessTextureToGlobalDescriptor(VulkanRendererContext* context, VkDescriptorSet bindlessTextureSet, uint32_t binding, AllocatedImage& image, VkSampler sampler, uint32_t index);

GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<UI::UIVertex> vertices, std::string name);
GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<Vertex> vertices, std::string name);
GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<VoxelVertex> vertices, std::string name);

// Batching
void buildDrawBatches(VulkanRendererContext* context, std::vector<Model>& models);
void buildUIDrawBatches(VulkanRendererContext* context, FixedArray<UI::UIWindowBatchCommands>& windowBatches);
std::vector<UI::UIElement> buildUIGeometry(VulkanRendererContext* context, FixedArray<UI::UIRenderCommand>& renderCommands, std::vector<UI::UIVertex>& vertices, std::vector<uint32_t>& indices);

// Updating
void rendererUpdate(VulkanRendererContext* context, float deltaTime);
void updateScene(VulkanRendererContext* context, float deltaTime);
void updateFontData(VulkanRendererContext* context);
void updateUIData(VulkanRendererContext* context);

// drawing
void rendererDraw(VulkanRendererContext* context);

void drawToGBuffer(VulkanRendererContext* context, VkCommandBuffer commandBuffer);
void resolveGBuffer(VulkanRendererContext* context, VkCommandBuffer commandBuffer);

void drawGeometry(VulkanRendererContext* context, VkCommandBuffer commandBuffer);
void drawUI(VulkanRendererContext* context, VkCommandBuffer commandBuffer);
void drawTerrain(VulkanRendererContext* context, VkCommandBuffer commandBuffer);

void rendererCleanup(VulkanRendererContext* context);

void immediateSubmit(VulkanRendererContext* context, std::function<void(VkCommandBuffer cmd)>&& function);

uint32_t registerImage(VulkanRendererContext* context, AllocatedImage* image);

void setPointerState(uint32_t windowId, float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);


// Voxel stuff
void initGBuffer(VulkanRendererContext* context);
void createGBuffer(VulkanRendererContext* context);
void destroyGBuffer(VulkanRendererContext* context);

}// namespace pm
