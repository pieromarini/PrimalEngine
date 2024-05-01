#pragma once

#include <array>
#include <iostream>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <format>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <vk_mem_alloc.h>

#include "vk_enum_string_helper.h"

#define VK_CHECK(x)                                                   \
	do {                                                                \
		VkResult err = x;                                                 \
		if (err) {                                                        \
			std::cout << std::format("Detected Vulkan error: {}", string_VkResult(err)); \
			abort();                                                        \
		}                                                                 \
	} while (0)
