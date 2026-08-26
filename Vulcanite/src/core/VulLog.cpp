#include <vector>
#include <filesystem>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "VulLog.h"

namespace Vulcanite {
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	
	void Log::Init() {
		// 日志文件放在可执行文件同级目录下的 log/ 文件夹,自动创建
		namespace fs = std::filesystem;
		fs::path logPath = fs::current_path() / "log";
		fs::create_directories(logPath);

		// Core logger: 独立 sinks(彩色控制台 + 文件)
		std::vector<spdlog::sink_ptr> coreSinks;
		coreSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		coreSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((logPath / "Vulcanite.log").string(), true));
		coreSinks[0]->set_pattern("%^[%T] %n: %v%$");
		coreSinks[1]->set_pattern("[%T] [%l] %n: %v");
		s_CoreLogger = std::make_shared<spdlog::logger>("Vulcanite", std::begin(coreSinks), std::end(coreSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		// Client logger: 独立 sinks,与 Core 互不干扰
		std::vector<spdlog::sink_ptr> clientSinks;
		clientSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		clientSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((logPath / "Vulcanite.log").string(), true));
		clientSinks[0]->set_pattern("%^[%T] %n: %v%$");
		clientSinks[1]->set_pattern("[%T] [%l] %n: %v");
		s_ClientLogger = std::make_shared<spdlog::logger>("APP", std::begin(clientSinks), std::end(clientSinks));
		spdlog::register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}

	void Log::Shutdown() {
		spdlog::shutdown();
	}
}