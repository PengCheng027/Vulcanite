#include "Core/Assert.h"

#include "Platform/Vulkan/VulkanMesh.h"

namespace Vulcanite {
	VulkanMesh::VulkanMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		:m_Vertices(vertices), m_Indices(indices) {

	}

	VulkanMesh::~VulkanMesh() {
		
	}

	const std::vector<Vertex>& VulkanMesh::GetVertices() const {
		return m_Vertices;
	}
		
	const std::vector<uint32_t>& VulkanMesh::GetIndices() const {
		return m_Indices;
	}

	uint32_t VulkanMesh::GetVertexCount() const {
		// 返回实际写入的顶点数(不是预分配容量 m_Vertices.size())
		return m_VertexCount;
	}

	uint32_t VulkanMesh::GetIndexCount() const {
		// 返回实际使用的索引数(不是预分配容量 m_Indices.size())
		return m_IndexCount;
	}
	
	void VulkanMesh::SetVertexCount(uint32_t vertexCount) {
		m_VertexCount = vertexCount;
	}

	void VulkanMesh::SetIndexCount(uint32_t indexCount) {
		m_IndexCount = indexCount;
	}

	void VulkanMesh::SetIndexDatas(uint32_t* ipIndexes, uint32_t indexCount) {
		if (indexCount == 0) {
			VULCANITE_CORE_ASSERT(false, "indexes count is 0!");
			return;
		}
		if (indexCount > m_Indices.size()) {
			VULCANITE_CORE_ASSERT(false, "index data exceeds mesh capacity!");
			return;
		}

		memcpy(m_Indices.data(), ipIndexes, indexCount * sizeof(uint32_t));
		m_IndexCount = indexCount;   // 同步实际数量
	}

	void VulkanMesh::SetVertexDatas(Vertex* ipVertexes, uint32_t vertexCount) {
		if (vertexCount == 0) {
			VULCANITE_CORE_ASSERT(false, "vertex count is 0!");
			return;
		}
		if (vertexCount > m_Vertices.size()) {
			VULCANITE_CORE_ASSERT(false, "vertex data exceeds mesh capacity!");
			return;
		}

		memcpy(m_Vertices.data(), ipVertexes, vertexCount * sizeof(Vertex));
		m_VertexCount = vertexCount;   // 同步实际数量
	}

	VkVertexInputBindingDescription VulkanMesh::GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanMesh::GetAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2, {});
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, m_Pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, m_Color);

		return attributeDescriptions;
	}
}