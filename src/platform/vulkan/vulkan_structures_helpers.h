#include <vk_types.h>

inline VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = flags;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	return createInfo;
}

inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count) {
	VkCommandBufferAllocateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	createInfo.commandPool = pool;
	createInfo.commandBufferCount = count;
	createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	return createInfo;
}
