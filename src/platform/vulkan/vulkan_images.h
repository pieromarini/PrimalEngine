#pragma once

#include <vulkan/vulkan.h>


void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
