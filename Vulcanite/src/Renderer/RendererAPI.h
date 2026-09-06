#pragma once

#include "Core/Base.h"

namespace Vulcanite {
	class RendererAPI {
	public:
		enum class API {
			None = 0, Vulkan = 1, D3D = 2
		};

	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;

		static API GetAPI();
		static Scope<RendererAPI> Create();
	private:
		static API s_API;
	};
}