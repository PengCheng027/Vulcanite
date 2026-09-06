#pragma once
#include <vector>
#include <vulkan/vulkan.h>

#include "Renderer/Mesh.h"

namespace Vulcanite {
	class VulkanMesh :public Mesh {
	public:
		VulkanMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		~VulkanMesh() override;

		const std::vector<Vertex>& GetVertices() const override;
		const std::vector<uint32_t>& GetIndices() const override;
		uint32_t GetVertexCount() const override;
		uint32_t GetIndexCount() const override;

		// Vulkan 专用(共 VulkanRenderer 用)
		VkBuffer GetVertexBuffer() const;
		VkBuffer GetIndexBuffer() const;
		uint32_t GetIndexCountVk() const;
	private:
		void CreateGPUBuffers();	// 上传到显存

		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_VertexMemory = VK_NULL_HANDLE;
		VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_IndexMemory = VK_NULL_HANDLE;
	};
}