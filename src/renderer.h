#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

#include "primal_window.h"
#include "platform/vulkan/validation.h"

namespace primal {

class Renderer {
  public:
	Renderer(Window* window) : m_window{ window } {}

	~Renderer() {
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}
		vkDestroyInstance(m_instance, nullptr);
	}

	void init() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("[VULKAN] Validation layers requested, but not available.");
		}
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Primal Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Primal";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// DEBUG: Get supported extensions from GLFW
		auto extensions = m_window->getGLFWExtensions(enableValidationLayers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("[VULKAN] Failed to create instance.");
		}

		// DEBUG: Check loaded extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> vulkanExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());
		std::cout << "[VULKAN] Available extensions:\n";

		for (const auto& extension : vulkanExtensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

		setupDebugMessages(m_instance, m_debugMessenger);
	}

	void render() {
	}

  private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	Window* m_window;
};

}// namespace primal
