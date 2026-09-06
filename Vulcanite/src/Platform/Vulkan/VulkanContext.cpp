#include <algorithm>
#include <string>

#include <cstring>

#include "Core/Assert.h"
#include "Core/VulLog.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Vulcanite {
	VulkanContext::VulkanContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle) {
	}

	VulkanContext::~VulkanContext() {
	#ifdef VULCANITE_DEBUG
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
	#endif
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanContext::Init(int width, int height) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		if (!m_WindowHandle) {
			m_WindowHandle = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
		}

		CreateInstance();

	#ifdef VULCANITE_DEBUG
		SetupDebugMessenger();
	#endif
	}

	void VulkanContext::CreateInstance() {
	#ifdef VULCANITE_DEBUG
		bool suppValidatLayer = CheckValidationLayerSupport();
		VULCANITE_CORE_ASSERT(suppValidatLayer, "Vulkan validation layer non't support");
	#endif

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulcanite";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		std::vector<const char*> extension = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extension.size());
		createInfo.ppEnabledExtensionNames = extension.data();

	#ifdef VULCANITE_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);

		createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
	#else
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	#endif

		VkResult createInsRes = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		VULCANITE_CORE_ASSERT(createInsRes == VK_SUCCESS, "vulkan create instance failed!");
	}

	bool VulkanContext::CheckValidationLayerSupport() const {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_ValidationLayers) {
			auto itLayer = std::find_if(availableLayers.begin(), availableLayers.end(), [layerName](const VkLayerProperties& comAvaLayre) {
				return strcmp(layerName, comAvaLayre.layerName) == 0;
			});

			if (itLayer != availableLayers.end()) {
				return true;
			}
		}

		return false;
	}

	std::vector<const char*> VulkanContext::GetRequiredExtensions() const {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	#ifdef VULCANITE_DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	#endif // VULCANITE_DEBUG

		return extensions;
	}

	void VulkanContext::SetupDebugMessenger() {
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		VkResult createDebugUtiMessEXT = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
		VULCANITE_CORE_ASSERT(createDebugUtiMessEXT == VK_SUCCESS, "failed to set up debug messenger!");
	}

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
		createInfo.pUserData = nullptr;
	}

	VkResult VulkanContext::CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {

		PFN_vkCreateDebugUtilsMessengerEXT func = 
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			VULCANITE_CORE_ASSERT(false, "GetInstanceProcAddr: createDebugMessenger,failed!");
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
		void* pUserData) {

		// 消息类型可能是多个标志的组合,逐个判断拼接成字符串
		std::string type;
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
			type += "General ";
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			type += "Validation ";
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			type += "Performance ";
		if (type.empty())
			type = "Unknown";

		// 按消息严重等级分发到对应日志级别
		switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				VULCANITE_CORE_TRACE("[{0}] {1}", type, pCallBackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				VULCANITE_CORE_INFO("[{0}] {1}", type, pCallBackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				VULCANITE_CORE_WARN("[{0}] {1}", type, pCallBackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				VULCANITE_CORE_ERROR("[{0}] {1}", type, pCallBackData->pMessage);
				break;
			default:
				VULCANITE_CORE_WARN("[{0}] Unknown severity: {1}", type, pCallBackData->pMessage);
				break;
		}

		return VK_FALSE;  // 返回 false,不终止应用(应用层自行决定是否崩溃)
	}

	void VulkanContext::DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		PFN_vkDestroyDebugUtilsMessengerEXT func = 
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
		else {
			VULCANITE_CORE_ASSERT(false, "GetInstanceProcAddr: destroy DebugMessenger,failed!");
		}
	}

	void* VulkanContext::GetWindowHandle() const {
		return m_WindowHandle;
	}
}
