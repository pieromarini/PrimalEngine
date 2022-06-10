#pragma once

#include <vulkan/vulkan.hpp>
#include "validation.h"
#include "../../primal_window.h"

namespace primal {

class VulkanContext {

  public:
	VulkanContext(Window* window) : m_window{ window } {}

	~VulkanContext() {
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}
		vkDestroyInstance(m_instance, nullptr);
	}

	void init() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("[VULKAN] Validation layers requested, but not available.");
		}
		vk::ApplicationInfo appInfo{};
		appInfo.sType = vk::StructureType::eApplicationInfo;
		appInfo.pApplicationName = "Primal Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Primal";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		vk::InstanceCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eInstanceCreateInfo;
		createInfo.pApplicationInfo = &appInfo;

		// DEBUG: Get supported extensions from GLFW
		auto extensions = m_window->getGLFWExtensions(enableValidationLayers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();


		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vk::createInstance(&createInfo, nullptr, &m_instance) != vk::Result::eSuccess) {
			throw std::runtime_error("[VULKAN] Failed to create instance.");
		}

		setupDebugMessenger(m_instance, m_debugMessenger);

		// DEBUG: Check loaded extensions
		/*
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<vk::ExtensionProperties> vulkanExtensions(extensionCount);
		auto res = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, vulkanExtensions.data());
		std::cout << "[VULKAN] Available extensions:\n";

		for (const auto& extension : vulkanExtensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}
		*/
	}

  private:
	Window* m_window;
	vk::Instance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
};

}// namespace primal
