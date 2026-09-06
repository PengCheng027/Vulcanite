#pragma once

#include "Core/Base.h"

namespace Vulcanite {
	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init(int width = 0, int height = 0) = 0;
		virtual void* GetWindowHandle() const = 0;

		static Scope<GraphicsContext> Create(void* windowHandle = nullptr);
	};
}