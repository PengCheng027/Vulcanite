#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Vulcanite {
	class Log {
	public:
		static void Init();
		static void Shutdown();
		inline static const std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static const std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

#ifdef VULCANITE_DEBUG
//Core log macros
#define VULCANITE_CORE_TRACE(...)		do { Vulcanite::Log::GetCoreLogger()->trace(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_INFO(...)		do { Vulcanite::Log::GetCoreLogger()->info(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_WARN(...)		do { Vulcanite::Log::GetCoreLogger()->warn(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_ERROR(...)		do { Vulcanite::Log::GetCoreLogger()->error(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_CRITICAL(...)	do { Vulcanite::Log::GetCoreLogger()->critical(__VA_ARGS__); } while (0)

//Client log macros
#define VULCANITE_CLIENT_TRACE(...)		do { Vulcanite::Log::GetClientLogger()->trace(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_INFO(...)		do { Vulcanite::Log::GetClientLogger()->info(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_WARN(...)		do { Vulcanite::Log::GetClientLogger()->warn(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_ERROR(...)		do { Vulcanite::Log::GetClientLogger()->error(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_CRITICAL(...)	do { Vulcanite::Log::GetClientLogger()->critical(__VA_ARGS__); } while (0)

#else
//Core log macros
#define VULCANITE_CORE_TRACE(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_INFO(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_WARN(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_ERROR(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CORE_CRITICAL(...)	do { (void)sizeof(__VA_ARGS__); } while (0)

//Client log macros
#define VULCANITE_CLIENT_TRACE(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_INFO(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_WARN(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_ERROR(...)		do { (void)sizeof(__VA_ARGS__); } while (0)
#define VULCANITE_CLIENT_CRITICAL(...)	do { (void)sizeof(__VA_ARGS__); } while (0)

#endif // VULCANITE_DEBUG


