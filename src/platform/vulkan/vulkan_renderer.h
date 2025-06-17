#pragma once

#include "assets/asset.h"
#include "assets/material_cache.h"
#include "renderer/material.h"
#include "terrain/voxel.h"
#include "ui/ui_manager.h"
#include "vulkan_loader.h"
#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vulkan/vulkan.h>

#include <ranges>
#include <vulkan/vulkan_core.h>

#include "camera.h"
#include "platform/vk_types.h"
#include "vulkan_descriptor.h"
#include "vulkan_texture.h"

#include "core/memory/arena.h"
#include "core/data_structures/stack.h"
#include "platform/window.h"
#include "ui/ui_types.h"
#include "ui/ui_widgets.h"


namespace pm {

struct UIVertex {
	vec2 position;
	vec2 uv;
	vec4 color;
};

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
	mat4 transform{};
	uint32_t materialIndex{};
	float padding[3]{ 0.0f, 0.0f, 0.0f };
};

struct alignas(16) UIDrawData {
	mat4 transform{};
	uint32_t materialIndex{};
	float padding[3]{ 0.0f, 0.0f, 0.0f };
};

struct alignas(16) FontDrawData {
	vec4 textColor;
};

struct alignas(16) ViewportDrawData {
	mat4 transform{};
	uint32_t textureIndex{};
};

struct alignas(16) UIMaterialData {
	vec4 backgroundColor;
	vec2 rectHalfSize;
	float borderThickness;
	float softness;
	float opacity;
	float padding;
	float cornerRadii[4];
};

struct MeshIndirectCommand {
	uint32_t drawId;
	VkDrawIndexedIndirectCommand command;
};

struct DrawBatchCommands {
	AllocatedBuffer buffer;
	uint32_t offset;
	uint32_t size;
	uint32_t stride;
};

enum DrawBatchType {
	DRAW_BATCH_MESH,
	DRAW_BATCH_UI,
	DRAW_BATCH_TEXT,
	DRAW_BATCH_VIEWPORT
};

struct DrawBatch {
	DrawBatchType type{};
	DrawBatchCommands commands{};
	GPUMeshBuffers meshBuffers{};

	MaterialInstance material;

	// TEMP
	std::vector<UIVertex> vertices{};
	std::vector<u32> indices{};

	std::vector<UIIndirectCommand> uiDrawCommands{};
	std::vector<UIDrawData> uiDrawData{};
	std::vector<UIMaterialData> uiMaterialData{};

	std::vector<UIIndirectCommand> textDrawCommands;
	std::vector<FontDrawData> textDrawData;
};

struct DrawBatchNode {
	DrawBatchNode* next{};
	DrawBatch drawBatch{};
};


// Group batches for a specific window
struct UIWindowBatch {
	PrimalWindow* window;
	DrawBatchNode* firstDrawBatch;
	DrawBatchNode* lastDrawBatch;
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
	double uiRenderTimeAvg{};
	double sceneUpdateTimeAvg{};
	double meshDrawTimeAvg{};
	double renderSubmitTimeAvg{};

	// 3D draw batch generation
	double entityFlattenTimeAvg{};
	double drawBatchGenerationTimeAvg{};

	// UI draw batch generation
	double uiBuildTimeAvg{};
	double uiSetupBuffersTimeAvg{};

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

StackDeclareNode(Transparency, f32);
StackDeclareNodeWithPointer(WindowBatch, UIWindowBatch);

struct FrameData {
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	VkFence renderFence;

	DeletionQueue deletionQueue;

	DescriptorAllocator frameDescriptor;

	std::vector<DrawBatch> drawBatches{};
	struct { WindowBatchNode* top; WindowBatchNode* free; bool autoPop; } uiWindowBatches;

	Arena* perFrameArena;
	Arena* perWindowArena;
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

struct UIContext;

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
	VkDescriptorSetLayout drawImageDescriptorLayout{};

	// Descriptor sets
	VkDescriptorSet drawImageDescriptors{};

	// BINDLESS DESCRIPTORS
	// Mesh textures
	VkDescriptorPool bindlessPool;
	VkDescriptorSetLayout bindlessTexturesSetLayout{};
	VkDescriptorSet bindlessTexturesDescriptorSet{};

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
	PrimalMaterial opaqueMaterial;
	PrimalMaterial doubleSidedMaterial;
	PrimalMaterial transparentMaterial;

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

	// Terrain test
	VoxelTerrain voxelTerrain;

	bool voxelWireframeActive{ false };
	MaterialPipeline voxelPipeline;
	MaterialPipeline voxelWireframePipeline;

	VkDescriptorSetLayout voxelDescriptorLayout;
	GPUMeshBuffers voxelMeshBuffers;
	AllocatedBuffer voxelDrawCommandsBuffer;
	uint32_t voxelDrawCommandsCount;
	TerrainParams terrainParams;

	// Material test
	PrimalMaterial uiMaterial;
	PrimalMaterial uiViewportMaterial;
	MaterialInstance uiViewportMaterialInstance;
	PrimalMaterial uiTextMaterial;

	std::unordered_map<std::string, std::pair<VkDescriptorSetLayout, VkDescriptorSet>> bindlessTextureArrays;

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

	// New UI system
	UIContext* mainUIContext;

	// TODO(piero): Should refactor this. Used to render a different layout to test full screen viewport rendering
	bool fullScreen{ false };

	StackDeclare(Transparency, transparency);
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
void cleanupTerrain(VulkanRendererContext* context);

void rendererInitDefaultData(VulkanRendererContext* context);

void initVulkan(VulkanRendererContext* context);
void initRenderTargets(VulkanRendererContext* context);
void initCommands(VulkanRendererContext* context);
void initSyncStructures(VulkanRendererContext* context);
void initDescriptors(VulkanRendererContext* context);
void initPipelines(VulkanRendererContext* context);
void initQueryPools(VulkanRendererContext* context);

void Renderer_initWindow(VulkanRendererContext* context, PrimalWindow* window);

void initFontData(VulkanRendererContext* context);
void initUI(VulkanRendererContext* context);
void initBindlessTextureDescriptor(VulkanRendererContext* context, VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet& descriptotSet);

void resizeSwapchain(VulkanRendererContext* context, PrimalWindow* window);
void resizeRenderTargets(VulkanRendererContext* context);

// specific pipelines
// TODO(piero): Init pipelines from config file
void initFontPipeline(VulkanRendererContext* context);
void initViewportPipeline(VulkanRendererContext* context);
void initUIPipeline(VulkanRendererContext* context);
void initMeshPipelines(VulkanRendererContext* context);

void initVoxelPipeline(VulkanRendererContext* context);

GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<UIVertex> vertices, std::string name);
GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<Vertex> vertices, std::string name);
GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<VoxelVertex> vertices, std::string name);

// Batching
void buildDrawBatches(VulkanRendererContext* context, std::vector<Model>& models);

// Updating
void Renderer_update(VulkanRendererContext* context, float deltaTime);
void updateScene(VulkanRendererContext* context, float deltaTime);
void updateFontData(VulkanRendererContext* context);
void updateUIData(VulkanRendererContext* context, f32 deltaTime);

// drawing
void Renderer_beginFrame(VulkanRendererContext* context);
void Renderer_endFrame(VulkanRendererContext* context);

void Renderer_beginWindow(VulkanRendererContext* context, PrimalWindow* window);
void Renderer_endWindow(VulkanRendererContext* context);
void Renderer_draw(VulkanRendererContext* context);

void drawToGBuffer(VulkanRendererContext* context, VkCommandBuffer commandBuffer);
void blitGBuffer(VulkanRendererContext* context, VkCommandBuffer commandBuffer);

void drawGeometry(VulkanRendererContext* context, VkCommandBuffer commandBuffer);
void drawUI(VulkanRendererContext* context, VkCommandBuffer commandBuffer);
void drawTerrain(VulkanRendererContext* context, VkCommandBuffer commandBuffer);

void rendererCleanup(VulkanRendererContext* context);

void immediateSubmit(VulkanRendererContext* context, std::function<void(VkCommandBuffer cmd)>&& function);

uint32_t registerImage(VulkanRendererContext* context, AllocatedImage* image);

void setPointerState(uint32_t windowId, float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);

// Voxel stuff
void setupVoxelRaycastRenderer(VulkanRendererContext* context);
void createGBuffer(VulkanRendererContext* context);
void destroyGBuffer(VulkanRendererContext* context);

// Material stuff
PrimalMaterial createMaterial(VulkanRendererContext* context, std::string_view materialConfig, bool flag = false);
void destroyMaterial(VulkanRendererContext* context, PrimalMaterial& material);

// API for UI rendering
DrawBatchNode* Renderer_getBatch(VulkanRendererContext* context, DrawBatchType type);
DrawBatchNode* Renderer_createBatch(VulkanRendererContext* context, DrawBatchType type);
void Renderer_pushRect(VulkanRendererContext* context, Rect2D rect, UIElement_RectStyleExt style);
void Renderer_pushText(VulkanRendererContext* context, vec2 offsetPosition, UIElement_TextExt* style);

// Stacks
f32 Renderer_pushTransparency(VulkanRendererContext* context, f32 value);
f32 Renderer_popTransparency(VulkanRendererContext* context);
f32 Renderer_topTransparency(VulkanRendererContext* context);

void Renderer_setupBuffers(VulkanRendererContext* context);

}// namespace pm
