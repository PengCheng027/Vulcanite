#pragma once
#include <vulkan/vulkan.h>

#include "Renderer/Pipeline.h"

namespace Vulcanite {
	class VulkanPipeline : public Pipeline {
	public:
		explicit VulkanPipeline(const PipelineSpe& spec);
		~VulkanPipeline() override;

		// ===== Pipeline 接口(平台无关) =====
		void Bind() const override;
		bool IsValid() const override;
		const PipelineSpe& GetSpec() const override;

		// ===== 后端内部接口:仅 Platform/Vulkan 下的代码使用 =====
		// 上层(Renderer2D / RendererAPI / SandBoxLayer)不要调用,出现即分层被破坏
		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
		VkPipelineLayout GetLayout() const { return m_PipelineLayout; }
		void BindDescriptorSet(VkCommandBuffer cmd, VkDescriptorSet set, uint32_t firstSet = 0) const;

	private:
		// 创建三步,顺序有依赖:描述符布局 → 管线布局 → 图形管线
		void CreateDescriptorSetLayout();
		void CreatePipelineLayout();
		void CreateGraphicsPipeline();

		void Destroy();

		// ===== 借用句柄(不拥有):生命周期由 VulkanContext 保证,这里只用于创建/销毁自己的对象 =====
		VkDevice m_Device = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		uint32_t m_Subpass = 0;

		// 规格:值语义保存,以后 Recreate(热重载 / 换 renderPass)直接复用
		PipelineSpe m_Spec;

		// ===== 本类拥有:析构时由 Destroy() 销毁 =====
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;
	};
}
