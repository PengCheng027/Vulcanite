#pragma once

#include <sstream>

#include "Core/KeyCodes.h"
#include "Events/Event.h"

namespace Vulcanite {
	class KeyEvent :public Event {
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(const KeyCode keyCode)
			:m_KeyCode(keyCode) {

		}

		KeyCode m_KeyCode;
	};

	class KeyPressedEvent :public KeyEvent {
	public:
		KeyPressedEvent(const KeyCode keyCode, bool isRepeate = false)
			:KeyEvent(keyCode), m_IsRepeate(isRepeate) {

		}

		bool IsRepeate() const { return m_IsRepeate; }

		std::string	ToString() const override {

			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << "(repeate = " << m_IsRepeate << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)

	private:
		bool m_IsRepeate;
	};

	class KeyReleasedEvent :public KeyEvent {
	public:
		KeyReleasedEvent(const KeyCode keyCode)
			:KeyEvent(keyCode) {

		}

		std::string ToString() const override {

			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent :public KeyEvent {
	public:
		KeyTypedEvent(const KeyCode keyCode)
			:KeyEvent(keyCode) {

		}

		std::string ToString() const override {

			std::stringstream ss;
			ss << "KeyTypedEvent" << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}