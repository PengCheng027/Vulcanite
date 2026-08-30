#include "Core/VulLog.h"
#include "Core/Core.h"
#include "Core/Timestep.h"

#include <GLFW/glfw3.h>

#include "Application.h"

namespace Vulcanite {
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		VULCANITE_CORE_ASSERT(!s_Instance, "Application already exists");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
	}

	Application:: ~Application() {
		s_Instance = nullptr;
	}

	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	// WindowsWindow's mData.EventCallback bind this function
	void Application::OnEvent(Event& e) {
		EventDispatcher dispathcher(e);
		// make sure the event is'n WindowCloseEvent
		dispathcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run() {
		// 帧计时:用 glfwGetTime() 计算每帧间隔(DeltaTime)
		while (m_Running) {
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timestep);

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}