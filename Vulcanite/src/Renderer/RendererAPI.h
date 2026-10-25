#pragma once

#include "Core/Base.h"

#include "Renderer/Mesh.h"
#include "Renderer/Pipeline.h"

namespace Vulcanite {
	class RendererAPI {
	public:
		enum class API {
			None = 0, Vulkan = 1, D3D = 2
		};

	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void Begin(const glm::mat4& viewProj) = 0;

		// 提交一次索引绘制:几何来自 Mesh,管线由调用方指定(一条批内只用一条管线)
		virtual void DrawIndex(const Mesh& mesh, const Ref<Pipeline>& pipeline) = 0;

		// 提交一次"原始顶点数据"的索引绘制:
		// 顶点字节流可以是任意布局,只要与管线的 PipelineSpe::Layout 匹配即可。
		// 这是"引擎支持多种顶点布局"的入口(不需要为每种布局新增 Mesh 子类)。
		virtual void DrawRaw(const void* vertexData, uint32_t vertexBytes,
			const uint32_t* indices, uint32_t indexCount,
			const Ref<Pipeline>& pipeline) = 0;

		static API GetAPI();
		static Scope<RendererAPI> Create();
	private:
		static API s_API;
	};
}
