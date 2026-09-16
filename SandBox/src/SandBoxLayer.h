#pragma once

#include "Core/Layer.h"
#include "Core/Timestep.h"
#include "Events/Event.h"

namespace Vulcanite {
	// 示例层:SandBox 应用的内容都放在这里
	class SandBoxLayer : public Layer {
	public:
		SandBoxLayer();
		virtual ~SandBoxLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
	};
}
