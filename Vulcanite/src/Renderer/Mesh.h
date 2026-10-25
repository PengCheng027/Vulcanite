#pragma once

#include <cstdint>
#include <vector>

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

		// ===== 批处理写入接口(供 Renderer2D 这类"收集者"使用)=====
		// 几何直接写进网格自己的存储:省掉"临时数组 + memcpy"那次拷贝,
		// 也让"这一批的数据存在哪"只有一个答案 —— Mesh。
		virtual uint32_t GetVertexCapacity() const = 0;
		virtual uint32_t GetIndexCapacity() const = 0;

		// 追加一个顶点,返回追加后的顶点数;容量不足时返回 0(不写入)
		virtual uint32_t AppendVertex(const Vertex& vertex) = 0;

		// 追加一段索引;容量不足返回 false(不写入)
		virtual bool AppendIndices(const uint32_t* indices, uint32_t indexCount) = 0;

		// 计数归零、保留容量:每批开始(或每帧开始)时调用
		virtual void ResetGeometry() = 0;

		// 工厂：由具体后端创建真正的网格
		static Ref<Mesh> Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	};
}