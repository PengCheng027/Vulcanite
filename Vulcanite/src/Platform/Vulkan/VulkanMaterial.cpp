#include "Platform/Vulkan/VulkanMaterial.h"

namespace Vulcanite {
	void VulkanMaterial::SetColor(const glm::vec4& color) {
		m_Color;
	}

	const glm::vec4& VulkanMaterial::GetColor() const {
		return m_Color;
	}

	const std::string& VulkanMaterial::GetName() const {
		return m_Name;
	};
}