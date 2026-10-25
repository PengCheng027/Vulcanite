#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

#include "Renderer/Mesh.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RendererAPI.h"

namespace Vulcanite {
	class VulkanRendererAPI : public RendererAPI {
	public:
		VulkanRendererAPI() = default;
		virtual ~VulkanRendererAPI() = default;

		void Init() override;
		void Begin(const glm::mat4& viewProject) override;
		void DrawIndex(const Mesh& mesh, const Ref<Pipeline>& pipeline) override;
		void DrawRaw(const void* vertexData, uint32_t vertexBytes,
			const uint32_t* indices, uint32_t indexCount,
			const Ref<Pipeline>& pipeline) override;
	};
}
