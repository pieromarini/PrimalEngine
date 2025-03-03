#pragma once

#include <vulkan/vulkan.h>

namespace pm {

void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);

void setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

}// namespace pm
