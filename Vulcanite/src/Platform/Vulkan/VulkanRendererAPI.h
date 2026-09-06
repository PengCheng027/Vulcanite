#pragma once

#include "Renderer/RendererAPI.h"

namespace Vulcanite {
	class VulkanRendererAPI : public RendererAPI {
	public:
		VulkanRendererAPI() = default;
		virtual ~VulkanRendererAPI() = default;
		void Init() override;
	};
}
