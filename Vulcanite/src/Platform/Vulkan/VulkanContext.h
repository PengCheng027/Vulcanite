#pragma once

#include <vector>
#include <optional>

// 必须在包含 glfw3.h 之前定义,glfw3.h 才会声明 Vulkan 相关函数(glfwCreateWindowSurface 等)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Renderer/GraphicsContext.h"

namespace Vulcanite {
	class VulkanContext : public GraphicsContext {
	public:
		VulkanContext(GLFWwindow* windowHandle = nullptr);
		virtual ~VulkanContext();

		void Init(int width = 0, int height = 0) override;
		void OnFrame() override;

		void* GetWindowHandle() const override;

	private:
		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			bool IsComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		void CreateInstance();
		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequiredExtensions() const;
		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void CreateSurface();

		void PickPhysicalDevice();
		void CreateLogicalDevice();
		bool IsDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		void CreateSwapChain();
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void CleanupSwapChain();

		void CreateImageViews();
		void CreateRenderPass();

		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>& code);

		void CreateCommandPool();

		void CreateFramebuffers();
		void CreateIndexBuffer();
		void CreateVertexBuffer();
		void CreateUniformBuffers();
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffer();
		void CreateSynObjects();

		void UpdateUniformBuffer(uint32_t currentImage);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void DrawFrame();


		static VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallBackData,
			void* pUserData);

		GLFWwindow* m_WindowHandle;
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkSurfaceKHR m_Surface;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device;

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		VkSwapchainKHR m_SwapChain;
		std::vector<VkImage> m_SwapChainImage;
		VkFormat m_SwapChainImageFormat;
		VkExtent2D m_SwapChainExtent;
		std::vector<VkImageView> m_SwapChainImageViews;
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		VkRenderPass m_RenderPass;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;

		VkCommandPool m_CommandPool;

		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;

		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		std::vector<VkCommandBuffer> m_CommandBuffers;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_ImagesFlight;

		int m_CurrentFrame = 0;

		static const int MAX_FRAMES_IN_FLIGHT = 2;

		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};
}
