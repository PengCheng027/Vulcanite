#pragma once

#include "Core/Base.h"

namespace Vulcanite {
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init(int width = 0, int height = 0) = 0;
		virtual void* GetWindowHandle() const = 0;

		// 每帧执行:渲染 + 呈现(由 Window::OnUpdate 调用)
		virtual void OnFrame() = 0;

		static Scope<GraphicsContext> Create(void* windowHandle = nullptr);
	};
}