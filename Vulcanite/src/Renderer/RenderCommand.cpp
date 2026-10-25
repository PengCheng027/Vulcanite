#include "Renderer/RenderCommand.h"

namespace Vulcanite {
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

	void RenderCommand::Init() {
		s_RendererAPI->Init();
	}

	void RenderCommand::Begin(const glm::mat4& viewProj) {
		s_RendererAPI->Begin(viewProj);
	}

	void RenderCommand::DrawIndex(const Mesh& mesh, const Ref<Pipeline>& pipeline) {
		s_RendererAPI->DrawIndex(mesh, pipeline);
	}
}
