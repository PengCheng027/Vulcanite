#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Core/VulLog.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer2D.h"

#include "SandBoxLayer.h"

namespace {
	// HSV → RGB(h ∈ [0,1] 色相,s 饱和度,v 明度,均 ∈ [0,1])
	glm::vec3 HsvToRgb(float h, float s, float v) {
		float r = v, g = v, b = v;
		float i = floorf(h * 6.0f);
		float f = h * 6.0f - i;
		float p = v * (1.0f - s);
		float q = v * (1.0f - s * f);
		float t = v * (1.0f - s * (1.0f - f));

		switch (static_cast<int>(i) % 6) {
			case 0: r = v; g = t; b = p; break;
			case 1: r = q; g = v; b = p; break;
			case 2: r = p; g = v; b = t; break;
			case 3: r = p; g = q; b = v; break;
			case 4: r = t; g = p; b = v; break;
			case 5: r = v; g = p; b = q; break;
		}
		return { r, g, b };
	}

	// 三次 Bézier 求值(演示:曲线只是折线的一种来源)
	glm::vec3 BezierPoint(const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3, float t) {
		const float u = 1.0f - t;
		return u * u * u * p0 + 3.0f * u * u * t * p1 + 3.0f * u * t * t * p2 + t * t * t * p3;
	}
}

namespace Vulcanite {
	SandBoxLayer::SandBoxLayer()
		: Layer("SandBoxLayer") {
	}

	SandBoxLayer::~SandBoxLayer() {
	}

	void SandBoxLayer::OnAttach() {
		VULCANITE_CLIENT_INFO("SandBoxLayer attached");

		// ===== 演示一:管线来自应用层,不碰 Vulkan =====
		// 与默认管线同顶点布局(Float2 + Float3 = 20 字节),只换成"反色 + 棋盘"的片元着色器。
		PipelineSpe altSpec;
		altSpec.VertexShaderPath = "assets/spir_v/shader_base.vert.spv";
		altSpec.FragmentShaderPath = "assets/spir_v/shader_alt.frag.spv";
		altSpec.Layout = BufferLayout{
			VertexAttribute(ShaderDataType::Float2, "m_Pos"),
			VertexAttribute(ShaderDataType::Float3, "m_Color")
		};
		altSpec.SampleCount = 1;

		m_AltPipeline = Pipeline::CreatePipeline(altSpec);

		VULCANITE_CLIENT_INFO("Second pipeline (same layout, alt fragment): {0}",
			m_AltPipeline != nullptr ? "created" : "FAILED");

		// ===== 演示二:不同的顶点布局 =====
		// 只有位置(2 个 float = 8 字节),没有颜色属性;着色器也从顶点取位置、片元输出固定颜色。
		// 这条管线与上面两条的 stride 不同,验证动态缓冲的"按字节追加"确实支持混合布局。
		PipelineSpe posOnlySpec;
		posOnlySpec.VertexShaderPath = "assets/spir_v/shader_posonly.vert.spv";
		posOnlySpec.FragmentShaderPath = "assets/spir_v/shader_posonly.frag.spv";
		posOnlySpec.Layout = BufferLayout{
			VertexAttribute(ShaderDataType::Float2, "m_Pos"),
			VertexAttribute(ShaderDataType::Float3, "m_Color")
		};
		posOnlySpec.Topology = PrimitiveTopology::LineList;
		posOnlySpec.SampleCount = 1;

		m_PosOnlyPipeline = Pipeline::CreatePipeline(posOnlySpec);

		VULCANITE_CLIENT_INFO("Third pipeline (pos-only layout, stride 8): {0}",
			m_PosOnlyPipeline != nullptr ? "created" : "FAILED");
	}

	void SandBoxLayer::OnDetach() {
		// 主动释放自己持有的管线引用:让资源在"窗口/设备销毁之前"按明确顺序归还,
		// 而不是依赖成员析构顺序。
		m_AltPipeline = nullptr;
		m_PosOnlyPipeline = nullptr;
		VULCANITE_CLIENT_INFO("SandBoxLayer detached");
	}

	void SandBoxLayer::OnUpdate(Timestep ts) {
		float aspect = 1280.0f / 720.0f;

		// bottom=1, top=-1 → 正交矩阵自带 Y 翻转(适配 Vulkan 的 NDC Y 轴向下)
		// near/far 用 0.1~100 是为了覆盖 lookAt 把物体推到相机空间 z=-2 的深度
		glm::mat4 proj = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

		glm::mat4 viewProject = proj;

		Renderer2D::BeginSence(viewProject);

		DrawDemoPolylines();
		// quad 默认是 1×1 世界单位,而网格间距只有 0.09:
		// 缩小到 0.08×0.08,让 400 个矩形彼此分开(否则会糊成一片)
		glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.08f));

		// 20×20 网格,色相沿对角线递进 → 彩虹斜条纹
		// 正交可见范围:x ∈ [-1.78, 1.78], y ∈ [-1, 1]
		for (int x = 0; x < 20; x++) {
			for (int y = 0; y < 20; y++) {
				// 第 10 列起换用第二条管线:Renderer2D 只结算"四边形批",
				// 折线批完全不受影响(两条批各有自己的存储和管线)。
				if (x == 10) {
					Renderer2D::SetQuadPipeline(m_AltPipeline);
				}

				float fx = -0.9f + x * 0.09f;
				float fy = -0.9f + y * 0.09f;

				float hue = (x + y) / 38.0f;   // 沿对角线 0 → 1 平滑递进
				glm::vec3 rgb = HsvToRgb(hue, 0.85f, 1.0f);

				Renderer2D::DrawQuad({ fx, fy, 0.0f }, { rgb.r, rgb.g, rgb.b, 1.0f }, transform);
			}
		}

		// 先画折线/曲线:它们和下面的 quad 落在同一个批里(同管线、同布局、同一次绘制)
		
		
		Renderer2D::EndSence();

		// ===== 演示二:用"不同顶点布局"的管线画一个三角形 =====
		// 顶点只有 vec2 位置(8 字节),由 RenderCommand::DrawRaw 直接提交;
		// 它和上面的彩虹网格共用同一份动态缓冲,只是起点字节偏移不同。
		
	}

	void SandBoxLayer::DrawDemoPolylines() {
		// ===== 验证 LineList 拓扑是否真的生效 =====
		// 折线批默认用"三角形展开"的默认管线(TriangleList)。这里把折线批换成
		// posOnly 管线(它的 Topology = LineList):
		//   - 若拓扑真的生效:折线会从"有宽度的三角形条带"变成 1px 硬件细线;
		//   - 颜色也会变成该管线片元着色器写死的橙色(说明确实换了管线)。
		Renderer2D::SetLinePipeline(m_PosOnlyPipeline);

		// 注意:LineList 模式下 width 会被忽略(硬件线宽固定 1px)。

		// 1) 闭合折线:三角形轮廓(3 个点 → 3 段 → 6 个索引)
		const glm::vec3 triangle[3] = {
			{ -1.60f,  0.30f, 0.0f },
			{ -1.10f,  0.30f, 0.0f },
			{ -1.35f, -0.05f, 0.0f },
		};
		Renderer2D::DrawPolyline(triangle, 3, 0.02f, { 1.0f, 1.0f, 1.0f, 1.0f }, true);

		// 2) 三次 Bézier 曲线:采样成 49 个点 → 48 段(曲线只是折线的一种来源)
		const glm::vec3 c0{ -1.70f, -0.45f, 0.0f };
		const glm::vec3 c1{ -1.50f,  0.10f, 0.0f };
		const glm::vec3 c2{ -1.25f, -0.95f, 0.0f };
		const glm::vec3 c3{ -1.00f, -0.45f, 0.0f };

		const int segments = 48;
		glm::vec3 curve[segments + 1];
		for (int i = 0; i <= segments; ++i) {
			curve[i] = BezierPoint(c0, c1, c2, c3, static_cast<float>(i) / static_cast<float>(segments));
		}
		Renderer2D::DrawPolyline(curve, static_cast<uint32_t>(segments + 1), 0.02f,
			{ 0.2f, 0.9f, 1.0f, 1.0f }, false);

		// 3) 单段线:一条横线
		Renderer2D::DrawLine({ -1.70f, -0.70f, 0.0f }, { -1.00f, -0.70f, 0.0f }, 0.01f,
			{ 1.0f, 0.5f, 0.1f, 1.0f });
	}

	void SandBoxLayer::OnEvent(Event& e) {
		// 示例:处理按键事件
		// EventDispatcher dispatcher(e);
		// dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(SandBoxLayer::OnKeyPressed));
	}
}
