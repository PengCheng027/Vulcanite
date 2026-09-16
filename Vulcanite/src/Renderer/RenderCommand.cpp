#include "Renderer/RenderCommand.h"

namespace Vulcanite {
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

	void RenderCommand::Init() {
		s_RendererAPI->Init();
	}

	void RenderCommand::DrawIndex(const Mesh& mesh) {
		s_RendererAPI->DrawIndex(mesh);
	}
}