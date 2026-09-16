#include "Core/VulLog.h"
#include "Renderer/Renderer2D.h"

#include "SandBoxLayer.h"

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
		// 每帧逻辑(动画、相机、脚本等)
		Renderer2D::BeginSence(glm::mat4(1.0f));
		// 正交可见范围:x ∈ [-1.78, 1.78], y ∈ [-1, 1],位置必须在范围内
		Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, glm::mat4(1.0f));
		Renderer2D::EndSence();
	}

	void SandBoxLayer::OnEvent(Event& e) {
		// 示例:处理按键事件
		// EventDispatcher dispatcher(e);
		// dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(SandBoxLayer::OnKeyPressed));
	}
}
