#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer2D.h"

namespace Vulcanite {
	Scope<Renderer::SceneData> Renderer::s_SceneData = nullptr;

	void Renderer::Init() {
		RenderCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::ShutDown() {
		Renderer2D::ShutDown();
	}

	void Renderer::BeginScene(const glm::mat4& viewProjectMatrix) {
		s_SceneData->ViewProjectionMatrix = viewProjectMatrix;
	}

	void Renderer::EndScene() {
	
	}

	void Renderer::Submit(const Ref<Mesh>& mesh, const Ref<Material>& material) {
		
	}
}