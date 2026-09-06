#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

#include "Renderer/Mesh.h"
#include "Renderer/Material.h"

namespace Vulcanite {

	class Renderer {
	public:
		static void Init();
		static void ShutDown();

		static void BeginScene(const glm::mat4& viewProjectMatrix);
		static void EndScene();

		static void Submit(const Ref<Mesh>& mesh,const Ref<Material>& material);
	private:

		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> s_SceneData;
	};
}