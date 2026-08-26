#include "core/VulLog.h"

int main() {
	// 彩色终端日志器
	Vulcanite::Log::Init();

	VULCANITE_CORE_TRACE("TRACE INFO {0},{1}", "testInfo Core", 0);
	VULCANITE_CORE_INFO("INFO INFO {0},{1}", "testInfo Core", 1);
	VULCANITE_CORE_WARN("INFO WRN {0},{1}", "testInfo Core", 2);
	VULCANITE_CORE_ERROR("INFO ERROR {0},{1}", "testInfo Core", 3);
	VULCANITE_CORE_CRITICAL("INFO CRITICAL {0},{1}", "testInfo Core", 4);

	VULCANITE_CLIENT_TRACE("TRACE INFO {0},{1}", "testInfo Client", 5);
	VULCANITE_CLIENT_INFO("INFO INFO {0},{1}", "testInfo Client", 6);
	VULCANITE_CLIENT_WARN("INFO WRN {0},{1}", "testInfo Client", 7);
	VULCANITE_CLIENT_ERROR("INFO ERROR {0},{1}", "testInfo Client", 8);
	VULCANITE_CLIENT_CRITICAL("INFO CRITICAL {0},{1}", "testInfo Client", 9);

	Vulcanite::Log::Shutdown();
	return 0;
}
