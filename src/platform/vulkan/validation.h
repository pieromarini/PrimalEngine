#pragma once

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <cstring>
#include <vector>

namespace primal {

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
[[maybe_unused]] constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

inline bool checkValidationLayerSupport() {
	uint32_t layerCount{};
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
  void* pUserData) {

	std::cerr << "[VULKAN] Validation Layer: " << pCallbackData->pMessage << '\n';

	return VK_FALSE;
}

inline VkResult CreateDebugUtilsMessengerEXT(vk::Instance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline void DestroyDebugUtilsMessengerEXT(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

inline void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT createInfo) {
	createInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
	createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	createInfo.pfnUserCallback = debugCallback;
}

inline void setupDebugMessenger(vk::Instance instance, VkDebugUtilsMessengerEXT& debugMessenger) {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("[VULKAN] Failed to set up debug messenger.");
	}
}

}// namespace primal
