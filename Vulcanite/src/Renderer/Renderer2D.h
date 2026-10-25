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

		// ===== 两条独立的批:四边形 / 折线 =====
		// 每条批自己收集、自己结算、自己指定管线:
		//  - 切换某条批的管线,只结算那一条(不会把另一条也切碎)
		//  - 同一帧里两条批各产生一条 DrawCommand,可以各用各的着色器
		static void SetQuadPipeline(const Ref<Pipeline>& pipeline);
		static void SetLinePipeline(const Ref<Pipeline>& pipeline);
		static Ref<Pipeline> GetQuadPipeline();
		static Ref<Pipeline> GetLinePipeline();

		// 四边形:世界坐标 = transform * 单位四边形 + position
		static void DrawQuad(const glm::vec3& position, const glm::vec4& color, const glm::mat4& transform);

		// ===== 线段 / 折线(进入折线批)=====
		// 在 CPU 端展开成"有宽度的三角形条带",因此同一条批内的折线共享一次绘制。
		// width 是世界单位(2D);点的 z 分量忽略。
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, float width, const glm::vec4& color);

		// closed = true 时首尾相连(画多边形轮廓/三角形边框)
		static void DrawPolyline(const glm::vec3* points, uint32_t pointCount, float width,
			const glm::vec4& color, bool closed = false);

	private:
		// 结算两条批(先四边形,后折线)
		static void Flush();
	};
}
