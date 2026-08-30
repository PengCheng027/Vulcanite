#include "Core/VulLog.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "WindowsWindow.h"

namespace Vulcanite {
	static bool sGLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char* description) {

		VULCANITE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window* Window::Create(const WindowProps& props) {
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) {
		Init(props);
	}

	WindowsWindow::~WindowsWindow() {

		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::Init(const WindowProps& props) {
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.Title = props.Title;

		VULCANITE_CORE_INFO("Creating window {0} ({1} {2})", props.Title, props.Width, props.Height);

		if (!sGLFWInitialized) {

			// TODD: glfwTerminate on system shutdown
			int success = glfwInit();
			VULCANITE_CORE_ASSERT(success, "Cloud not initialize GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);

			sGLFWInitialized = true;

		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(),
			nullptr, nullptr); // this function will create a window pointer and context

		glfwMakeContextCurrent(m_Window); // this function make the window in current context
		//int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		//HZ_CORE_ASSERT(status, "Faild to initialize Glad!");
		glfwSetWindowUserPointer(m_Window, &m_Data); // set some information of window to use later
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
			// get the information about this window
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			//	using this function point, call Event.this funtion point binded by 
			//	Window::SetEventCallback(const EventCallbackFn& callback)
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {

				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int character) {

			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(character);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int	action, int mods) {

			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {

				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});
	}

	void WindowsWindow::Shutdown() {

		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnUpdate() {
		// this function will process events in queue
		glfwPollEvents();

		glfwSwapBuffers(m_Window);
	}

	void WindowsWindow::SetVSync(bool enable) {

		if (enable) {
			// wait for SwapBuffer is ready to swap
			glfwSwapInterval(1);
		}
		else {
			// Swap buffer immediately, event the buffer is'n ready
			glfwSwapInterval(0);
		}

		m_Data.VSync = enable;
	}

	bool WindowsWindow::IsVSync() const {

		return m_Data.VSync;
	}
}