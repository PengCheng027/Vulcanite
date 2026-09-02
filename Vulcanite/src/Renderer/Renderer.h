#pragma once

#include <glm/glm.hpp>
namespace Vulcanite {

	class Renderer {
	public:
		static void Init();
		static void ShutDown();

		static void BeginScene();
		static void EndScene();

		static void Submit();
	private:

		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};


	};
}