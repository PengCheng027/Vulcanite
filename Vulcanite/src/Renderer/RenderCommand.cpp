#include "Renderer/RenderCommand.h"

namespace Vulcanite {
	Scope<RendererAPI> RenderCommand::s_RendererAPI = RendererAPI::Create();

	void RenderCommand::Init() {
		s_RendererAPI->Init();
	}
}