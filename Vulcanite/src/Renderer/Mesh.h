#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

namespace Vulcanite {
	struct Vertex {
		glm::vec2 m_Pos;
		glm::vec3 m_Color;
	};

	class Mesh {
	public:
		virtual ~Mesh() = default;

		//几何数据
		virtual const std::vector<Vertex>& GetVertices() const = 0;
		virtual const std::vector<uint32_t>& GetIndices() const = 0;

		virtual uint32_t GetVertexCount() const = 0;
		virtual uint32_t GetIndexCount() const = 0;

		virtual void SetVertexCount(uint32_t vertexCount) = 0;
		virtual void SetIndexCount(uint32_t indexCount) = 0;

		virtual void SetVertexDatas(Vertex* ipVertexes, uint32_t vertexCount) = 0;
		virtual void SetIndexDatas(uint32_t* ipIndexes, uint32_t indexCount) = 0;
		// 工厂：由具体后端创建真正的网格
		static Ref<Mesh> Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	};
}