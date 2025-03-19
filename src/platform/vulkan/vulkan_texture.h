#include <string>
#include <vulkan/vulkan.h>

#include "vk_types.h"

#include "ktx.h"

namespace pm {

void getValidFilters(VkPhysicalDevice physicalDevice, VkFormat format, VkFilter* filter, VkSamplerMipmapMode* mipmapMode);

AllocatedBuffer createBuffer(std::string name, size_t allocSize, VmaAllocator allocator, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
void destroyBuffer(VmaAllocator allocator, const AllocatedBuffer& buffer);

std::optional<AllocatedImage> createKTX2Image(std::string_view imageName, VkDevice device, VkCommandPool commandPool, VkQueue copyQueue, VmaAllocator allocator, void* imageData, uint32_t imageDataSize, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout);

uint32_t getMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound);

class Texture {
public:
	void updateDescriptor();
	void destroy(VkDevice device);
	ktxResult loadKTXFile(std::string filename, ktxTexture** target);

	VkImage image;
	VkImageLayout imageLayout;
	VkDeviceMemory deviceMemory;
	VkImageView view;
	uint32_t width, height;
	uint32_t mipLevels;
	uint32_t layerCount;
	VkDescriptorImageInfo descriptor;
	VkSampler sampler;
};

class Texture2D : public Texture {
public:
	void loadFromFile(
		std::string filename,
		VkFormat format,
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool,
		VkQueue copyQueue,
		float maxAnisotropy,
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};

}// namespace pm
