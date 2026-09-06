#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"

namespace Vulcanite {
	Scope<Renderer::SceneData> Renderer::s_SceneData = nullptr;

	void Renderer::Init() {
		RenderCommand::Init();
	}

	void Renderer::ShutDown() {
	
	}

	void Renderer::BeginScene(const glm::mat4& viewProjectMatrix) {
		s_SceneData->ViewProjectionMatrix = viewProjectMatrix;
	}

	void Renderer::EndScene() {
	
	}

	void Renderer::Submit(const Ref<Mesh>& mesh, const Ref<Material>& material) {
		
	}
}