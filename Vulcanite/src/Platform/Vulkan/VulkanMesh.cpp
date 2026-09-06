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

	uint32_t VulkanMesh::GetVertexCount()const {
		return m_Vertices.size();
	}

	uint32_t VulkanMesh::GetIndexCount() const {
		return m_Indices.size();
	}

	VkBuffer VulkanMesh::GetVertexBuffer() const {
		return m_VertexBuffer;
	}

	VkBuffer VulkanMesh::GetIndexBuffer() const {
		return m_IndexBuffer;
	}

	uint32_t VulkanMesh::GetIndexCountVk() const {
		return 0;
	}

	void VulkanMesh::CreateGPUBuffers() {
		
	}
}