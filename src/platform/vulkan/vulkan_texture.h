#include <string>
#include <vulkan/vulkan.h>

#include "ktx.h"
#include "ktxvulkan.h"

namespace pm {

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
		VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};

}// namespace pm
