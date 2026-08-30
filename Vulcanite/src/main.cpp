#include "Core/VulLog.h"
#include "Application.h"

int main() {
	// 彩色终端日志器
	Vulcanite::Log::Init();

	Vulcanite::Application app;
	app.Run();

	Vulcanite::Log::Shutdown();
	return 0;
}
