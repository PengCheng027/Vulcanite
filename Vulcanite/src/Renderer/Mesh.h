#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

namespace Vulcanite {
	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	class Mesh {
	public:
		virtual ~Mesh() = default;

		//几何数据
		virtual const std::vector<Vertex>& GetVertices() const = 0;
		virtual const std::vector<uint32_t>& GetIndices() const = 0;

		virtual uint32_t GetVertexCount() const = 0;
		virtual uint32_t GetIndexCount() const = 0;

		// 工厂：由具体后端创建真正的网格
		static Ref<Mesh> Create(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	};
}