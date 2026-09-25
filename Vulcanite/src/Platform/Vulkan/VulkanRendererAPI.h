#pragma once

#include <glm/glm.hpp>

#include "Renderer/Mesh.h"
#include "Renderer/RendererAPI.h"

namespace Vulcanite {
	class VulkanRendererAPI : public RendererAPI {
	public:
		VulkanRendererAPI() = default;
		virtual ~VulkanRendererAPI() = default;
		void Init() override;
		void Begin(const glm::mat4& viewProject) override;
		void DrawIndex(const Mesh& mesh) override;
	};
}
