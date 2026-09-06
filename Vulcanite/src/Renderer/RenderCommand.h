#pragma once

#include "Core/Base.h"

#include "Renderer/RendererAPI.h"

namespace Vulcanite {
	class RenderCommand {
	public:
		static void Init();

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}