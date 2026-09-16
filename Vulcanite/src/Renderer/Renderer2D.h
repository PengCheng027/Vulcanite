#pragma once

#include <glm/glm.hpp>

namespace Vulcanite {
	class Renderer2D {
	public:
		static void Init();
		static void ShutDown();

		static void BeginSence(const glm::mat4& viewProjectMatrix);
		static void EndSence();

		static void DrawQuad(const glm::vec3& position, const glm::vec4& color, const glm::mat4& transform);
	};
}