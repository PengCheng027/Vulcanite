#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Vulcanite {
	// Vulkan 的实例/调试/surface/设备等上下文初始化已迁至 VulkanContext,
	// VulkanRendererAPI 专注绘制命令(清屏、绑定管线、绘制等),后续填充
	void VulkanRendererAPI::Init() {
	}

	void VulkanRendererAPI::Begin(const glm::mat4& viewProject) {
		VulkanContext::Get()->SetUniBufferObject({ viewProject });
	}

	void VulkanRendererAPI::DrawIndex(const Mesh& mesh) {
		const auto& vertices = mesh.GetVertices();
		const auto& indices = mesh.GetIndices();

		uint32_t vertexCount = mesh.GetVertexCount();
		uint32_t indexCount = mesh.GetIndexCount();

		VulkanContext::Get()->UploadDynamicGeometry(
			vertices.data(), vertexCount * sizeof(Vertex),
			indices.data(), indexCount * sizeof(uint32_t));
	}
}
