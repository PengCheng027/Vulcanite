#pragma once

#include <glm/glm.hpp>

#include "Core/Base.h"

namespace Vulcanite {
	class Pipeline;

	class Renderer2D {
	public:
		static void Init();
		static void ShutDown();

		static void BeginSence(const glm::mat4& viewProjectMatrix);
		static void EndSence();

		// 切换后续绘制使用的管线。切换前会先结算当前批,
		// 因此一个批(一次绘制命令)里只可能有一条管线。
		static void SetPipeline(const Ref<Pipeline>& pipeline);
		static Ref<Pipeline> GetPipeline();

		// 四边形:世界坐标 = transform * 单位四边形 + position
		static void DrawQuad(const glm::vec3& position, const glm::vec4& color, const glm::mat4& transform);

		// ===== 线段 / 折线 =====
		// 在 CPU 端展开成"有宽度的三角形条带",因此和 quad 共用同一个批、同一条管线、
		// 同一次 vkCmdDrawIndexed —— 这就是折线能参与批量绘制的原因。
		// width 是世界单位(2D);点的 z 分量忽略。
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, float width, const glm::vec4& color);

		// closed = true 时首尾相连(画多边形轮廓/三角形边框)
		static void DrawPolyline(const glm::vec3* points, uint32_t pointCount, float width,
			const glm::vec4& color, bool closed = false);

	private:
		// 把当前收集到的几何结算成一条绘制命令(上传几何 + 记录管线)
		static void Flush();

		// 容量不足时先 Flush;单次请求超过整批容量则失败(返回 false)
		static bool EnsureCapacity(uint32_t vertexCount, uint32_t indexCount);

		static void EmitVertex(const glm::vec2& position, const glm::vec4& color);
		static void EmitTriangle(uint32_t a, uint32_t b, uint32_t c);
	};
}
