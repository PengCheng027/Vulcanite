#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	// 彩色终端日志器
	auto console = spdlog::stdout_color_mt("console");

	console->info("hello vulcanite engine");
	console->warn("this is a warning");
	console->error("this is an error");

	spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");

	console->info("colored pattern test");

	return 0;
}
