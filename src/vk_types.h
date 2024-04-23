#pragma once

#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <format>

#include "vk_enum_string_helper.h"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#define VK_CHECK(x)                                                   \
	do {                                                                \
		VkResult err = x;                                                 \
		if (err) {                                                        \
			std::format("Detected Vulkan error: {}", string_VkResult(err)); \
			abort();                                                        \
		}                                                                 \
	} while (0)
