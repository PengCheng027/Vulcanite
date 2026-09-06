#pragma once
#include <vulkan/vulkan.h>

#include <string>

#include <glm/glm.hpp>

#include "Renderer/Material.h"

namespace Vulcanite {
	class VulkanMaterial :public Material {
	public:
		VulkanMaterial() = default;
		~VulkanMaterial() = default;
		void SetColor(const glm::vec4& color) override;
		const glm::vec4& GetColor() const override;

		const std::string& GetName() const override;

	private:
		glm::vec4 m_Color = { 1.0f,1.0f,1.0f,1.0f };
		std::string m_Name = "";

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
	};
}