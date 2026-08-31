#pragma once

namespace Vulcanite {
	class Renderer {
	public:
		static void Init();
		static void ShutDown();

		static void BeginScene();
		static void EndScene();

		static void Submit();
	};
}