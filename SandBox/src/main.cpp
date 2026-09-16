#include "Core/VulLog.h"
#include "Application.h"

#include "SandBoxLayer.h"

int main() {
	Vulcanite::Log::Init();

	Vulcanite::Application app;
	app.PushLayer(new Vulcanite::SandBoxLayer());
	app.Run();

	Vulcanite::Log::Shutdown();
	return 0;
}
