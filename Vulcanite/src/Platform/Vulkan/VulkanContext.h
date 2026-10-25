#pragma once

#include <vector>
#include <optional>

#include <glm/glm.hpp>
// 必须在包含 glfw3.h 之前定义,glfw3.h 才会声明 Vulkan 相关函数(glfwCreateWindowSurface 等)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Core/Base.h"

#include "Renderer/DrawCommand.h"
#include "Renderer/GraphicsContext.h"

namespace Vulcanite {
	class VulkanPipeline;   // 具体实现,定义在 Platform/Vulkan/VulkanPipeline.h

	struct UniformBufferObject {
		alignas(16) glm::mat4 viewProj;
	};

	class VulkanContext : public GraphicsContext {
	public:
		VulkanContext(GLFWwindow* windowHandle = nullptr);
		virtual ~VulkanContext();

		void Init(int width = 0, int height = 0) override;
		void OnFrame() override;

		void* GetWindowHandle() const override;

		// ===== 动态批缓冲:追加写入 =====
		// 一次绘制一份几何,同一帧的多条绘制依次追加到同一份缓冲。
		// 顶点用**字节偏移**定位(不假设 stride),因此不同顶点布局可以共存;
		// outVertexByteOffset / outFirstIndex 供 vkCmdBindVertexBuffers 与 vkCmdDrawIndexed 使用。
		// 返回 false 表示容量不足(本次未写入)。
		bool AppendDynamicGeometry(const void* vertexData, VkDeviceSize vertexBytes,
			const void* indexData, VkDeviceSize indexBytes,
			uint32_t& outVertexByteOffset, uint32_t& outFirstIndex);

		void SetUniBufferObject(const UniformBufferObject& uniBufferObject);

		// ===== 绘制命令列表:上层填充,RecordCommandBuffer 逐条执行 =====
		void PushDrawCommand(const DrawCommand& command);
		void ClearDrawCommands();
		const std::vector<DrawCommand>& GetDrawCommands() const;

		VkBuffer GetDynamicVertexBuffer() const;
		VkBuffer GetDynamicIndexBuffer() const;

		VkDevice GetDevice() const;
		VkRenderPass GetRenderPass() const;
		VkCommandBuffer GetCurrentCommandBuffer() const;

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

		// 帧级描述符布局(binding 0 = 相机 UBO):由上下文拥有,与管线解耦。
		// 必须与 PipelineSpe::DefaultFrameBindings() 定义一致,两者才能共用同一份描述符集。
		void CreateFrameDescriptorSetLayout();

		void CreateCommandPool();

		void CreateFramebuffers();
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

		// 帧级描述符布局:管线只是"引用"同一定义的布局,这里由上下文拥有,
		// 因此管线可以从上下文里挪走(现在由 Renderer2D 创建并持有)。
		VkDescriptorSetLayout m_FrameDescriptorSetLayout = VK_NULL_HANDLE;

		VkCommandPool m_CommandPool;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;

		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;

		std::vector<VkCommandBuffer> m_CommandBuffers;
		VkCommandBuffer m_CurrentCommandBuffer = VK_NULL_HANDLE;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkFence> m_InFlightFences;

		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_ImagesFlight;

		// ===== 动态批缓冲(每帧一份, host-visible + 常驻映射, CPU 直接 memcpy) =====
		// 顶点缓冲按"字节竞技场"使用:不假设 stride,任何顶点布局的几何都能依次追加。
		std::vector<VkBuffer> m_DynamicVertexBuffers;
		std::vector<VkDeviceMemory> m_DynamicVertexMemory;
		std::vector<void*> m_DynamicVertexMapped;

		std::vector<VkBuffer> m_DynamicIndexBuffers;
		std::vector<VkDeviceMemory> m_DynamicIndexMemory;
		std::vector<void*> m_DynamicIndexMapped;

		std::vector<uint32_t> m_DynamicVertexBytesUsed;   // 每帧已写入的顶点字节数
		std::vector<uint32_t> m_DynamicIndexCounts;       // 每帧已写入的索引数

		VkDeviceSize m_DynamicVertexCapacityBytes = 0;
		VkDeviceSize m_DynamicIndexCapacityBytes = 0;

		// 本帧的绘制命令列表:Begin 时清空,RecordCommandBuffer 遍历执行
		std::vector<DrawCommand> m_DrawCommands;

		// 容量按字节计:4 MB 顶点足以容纳约 20 万个 20 字节顶点(远超当前需求)
		static constexpr uint32_t MAX_DYNAMIC_VERTEX_BYTES = 4 * 1024 * 1024;
		static constexpr uint32_t MAX_DYNAMIC_INDICES = 150000;

		int m_CurrentFrame = 0;

		UniformBufferObject m_UniBufferObject = {};

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
