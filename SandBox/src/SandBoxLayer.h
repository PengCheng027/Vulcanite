#pragma once

#include "Core/Base.h"
#include "Core/Layer.h"
#include "Core/Timestep.h"
#include "Events/Event.h"

namespace Vulcanite {
	class Pipeline;

	// 示例层:SandBox 应用的内容都放在这里
	class SandBoxLayer : public Layer {
	public:
		SandBoxLayer();
		virtual ~SandBoxLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;

	private:
		// 演示一:与默认管线同布局、只换片元着色器(反色 + 棋盘)
		Ref<Pipeline> m_AltPipeline = nullptr;

		// 演示二:不同顶点布局(只有 vec2 位置,stride 8)+ 另一个着色器
		Ref<Pipeline> m_PosOnlyPipeline = nullptr;

		// 演示:折线/曲线轮廓也进 Renderer2D 的批(三角形展开)
		void DrawDemoPolylines();
	};
}
