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

		void SetVertexCount(uint32_t vertexCount) override;
		void SetIndexCount(uint32_t indexCount) override;

		void SetIndexDatas(uint32_t* ipIndexes, uint32_t indexCount) override;
		void SetVertexDatas(Vertex* ipVertexes, uint32_t vertexCount) override;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	private:

		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		uint32_t m_VertexCount = 0;
		uint32_t m_IndexCount = 0;
	};
}