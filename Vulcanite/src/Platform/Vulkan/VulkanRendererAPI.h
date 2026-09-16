#pragma once

#include "Renderer/Mesh.h"
#include "Renderer/RendererAPI.h"

namespace Vulcanite {
	class VulkanRendererAPI : public RendererAPI {
	public:
		VulkanRendererAPI() = default;
		virtual ~VulkanRendererAPI() = default;
		void Init() override;
		void DrawIndex(const Mesh& mesh) override;
	};
}
