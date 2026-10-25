#pragma once

#include "Core/Base.h"

#include "Renderer/Mesh.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RendererAPI.h"

namespace Vulcanite {
	class RenderCommand {
	public:
		static void Init();
		static void Begin(const glm::mat4& viewProj);

		static void DrawIndex(const Mesh& mesh, const Ref<Pipeline>& pipeline);

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};
}
