#pragma once

#include <memory>

#include "Core/Window.h"
#include "Core/Layer.h"
#include "Core/LayerStack.h"

#include "Events/ApplicationEvent.h"

namespace Vulcanite {
	class Application {
	public:
		Application();
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		float m_LastFrameTime = 0.0f;
		LayerStack m_LayerStack;

		static Application* s_Instance;
	};
}