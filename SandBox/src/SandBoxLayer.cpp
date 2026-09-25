#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

#include "Core/VulLog.h"
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
}

namespace Vulcanite {
	SandBoxLayer::SandBoxLayer()
		: Layer("SandBoxLayer") {
	}

	SandBoxLayer::~SandBoxLayer() {
	}

	void SandBoxLayer::OnAttach() {
		VULCANITE_CLIENT_INFO("SandBoxLayer attached");
	}

	void SandBoxLayer::OnDetach() {
		VULCANITE_CLIENT_INFO("SandBoxLayer detached");
	}

	void SandBoxLayer::OnUpdate(Timestep ts) {
		float aspect = 1280.0f / 720.0f;

		// bottom=1, top=-1 → 正交矩阵自带 Y 翻转(适配 Vulkan 的 NDC Y 轴向下)
		// near/far 用 0.1~100 是为了覆盖 lookAt 把物体推到相机空间 z=-2 的深度
		glm::mat4 proj = glm::ortho(-aspect, aspect, 1.0f, -1.0f, -1.0f, 1.0f);

		glm::mat4 viewProject = proj;

		Renderer2D::BeginSence(viewProject);

		// quad 默认是 1×1 世界单位,而网格间距只有 0.09:
		// 缩小到 0.08×0.08,让 400 个矩形彼此分开(否则会糊成一片)
		glm::mat4 transform = glm::scale(glm::mat4(1.0f), glm::vec3(0.08f));

		// 20×20 网格,色相沿对角线递进 → 彩虹斜条纹
		// 正交可见范围:x ∈ [-1.78, 1.78], y ∈ [-1, 1]
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 20; x++) {
				float fx = -0.9f + x * 0.09f;
				float fy = -0.9f + y * 0.09f;

				float hue = (x + y) / 38.0f;   // 沿对角线 0 → 1 平滑递进
				glm::vec3 rgb = HsvToRgb(hue, 0.85f, 1.0f);

				Renderer2D::DrawQuad({ fx, fy, 0.0f }, { rgb.r, rgb.g, rgb.b, 1.0f }, transform);
			}
		}

		Renderer2D::EndSence();
	}

	void SandBoxLayer::OnEvent(Event& e) {
		// 示例:处理按键事件
		// EventDispatcher dispatcher(e);
		// dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(SandBoxLayer::OnKeyPressed));
	}
}
