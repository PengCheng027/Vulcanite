#pragma once

namespace Vulcanite {
	// 封装一帧的时间(秒),轻量浮点包装
	class Timestep {
	public:
		Timestep(float time = 0.0f)
			: m_Time(time) {
		}

		// 隐式转换为 float,方便直接参与计算
		operator float() const { return m_Time; }

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }
	private:
		float m_Time;
	};
}
