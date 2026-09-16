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

		// ===== 动态批缓冲接口(CPU 每帧写入几何数据,host-visible 常驻映射) =====
		// 上传本帧收集到的顶点/索引数据(字节级接口,避免头文件暴露顶点类型)
		// 返回 false 表示超出容量(数据未写入)
		bool UploadDynamicGeometry(const void* vertexData, VkDeviceSize vertexBytes,
			const void* indexData, VkDeviceSize indexBytes);

		VkBuffer GetDynamicVertexBuffer() const;
		VkBuffer GetDynamicIndexBuffer() const;
		uint32_t GetDynamicIndexCount() const;

		static VulkanContext* Get();
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

		// 动态批缓冲:创建(含常驻映射)与销毁
		void CreateDynamicBuffers();
		void DestroyDynamicBuffers();

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

		// ===== 动态批缓冲(每帧一份, host-visible + 常驻映射, CPU 直接 memcpy) =====
		std::vector<VkBuffer> m_DynamicVertexBuffers;
		std::vector<VkDeviceMemory> m_DynamicVertexMemory;
		std::vector<void*> m_DynamicVertexMapped;

		std::vector<VkBuffer> m_DynamicIndexBuffers;
		std::vector<VkDeviceMemory> m_DynamicIndexMemory;
		std::vector<void*> m_DynamicIndexMapped;

		std::vector<uint32_t> m_DynamicIndexCounts;   // 每帧实际写入的索引数

		VkDeviceSize m_DynamicVertexCapacityBytes = 0;
		VkDeviceSize m_DynamicIndexCapacityBytes = 0;

		// 容量按 Renderer2D 目标(MaxQuads = 20000,每 quad 4 顶点 6 索引)留足余量
		static constexpr uint32_t MAX_DYNAMIC_VERTICES = 100000;
		static constexpr uint32_t MAX_DYNAMIC_INDICES = 150000;

		int m_CurrentFrame = 0;

		static const int MAX_FRAMES_IN_FLIGHT = 2;

		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> m_DeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		static VulkanContext* s_Instance;
	};
}
