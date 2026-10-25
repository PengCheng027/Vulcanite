#include "Core/Assert.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Vulcanite {
	// Vulkan 的实例/调试/surface/设备等上下文初始化在 VulkanContext,
	// 这里只负责把上层提交的绘制转换成"绘制命令"(几何追加写入 + 带管线)。
	void VulkanRendererAPI::Init() {
	}

	void VulkanRendererAPI::Begin(const glm::mat4& viewProject) {
		VulkanContext* context = VulkanContext::Get();
		if (context == nullptr) {
			return;
		}

		context->SetUniBufferObject({ viewProject });
		context->ClearDrawCommands();   // 新的一帧从空命令列表开始
	}

	void VulkanRendererAPI::DrawIndex(const Mesh& mesh, const Ref<Pipeline>& pipeline) {
		const uint32_t vertexCount = mesh.GetVertexCount();
		const uint32_t indexCount = mesh.GetIndexCount();
		if (vertexCount == 0 || indexCount == 0) {
			return;
		}

		// Mesh 的顶点是固定的 Vertex 布局(位置 + 颜色),按它的 stride 上传
		DrawRaw(mesh.GetVertices().data(),
			vertexCount * static_cast<uint32_t>(sizeof(Vertex)),
			mesh.GetIndices().data(), indexCount, pipeline);
	}

	void VulkanRendererAPI::DrawRaw(const void* vertexData, uint32_t vertexBytes,
		const uint32_t* indices, uint32_t indexCount,
		const Ref<Pipeline>& pipeline) {
		VulkanContext* context = VulkanContext::Get();
		if (context == nullptr || pipeline == nullptr) {
			return;
		}
		if (vertexData == nullptr || vertexBytes == 0 || indices == nullptr || indexCount == 0) {
			return;
		}

		DrawCommand command;
		command.Pipeline = pipeline;
		command.IndexCount = indexCount;

		const bool appended = context->AppendDynamicGeometry(
			vertexData, static_cast<VkDeviceSize>(vertexBytes),
			indices, static_cast<VkDeviceSize>(indexCount) * sizeof(uint32_t),
			command.VertexByteOffset, command.FirstIndex);

		if (!appended) {
			VULCANITE_CORE_ERROR("本帧动态几何缓冲已满,丢弃一次绘制(索引 {0} 个)", indexCount);
			return;
		}

		context->PushDrawCommand(command);
	}
}
