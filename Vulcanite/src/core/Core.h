#pragma once

#define BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define BIT(x) (1 << x)

#ifdef VULCANITE_ENABLE_ASSERTS
	#define VULCANITE_ASSERT(x,...)			do{ if(!(x)){VULCANITE_CLIENT_ERROR("Assertion Failed: {0}",__VA_ARGS__);__debugbreak();}}while(0)
	#define VULCANITE_CORE_ASSERT(x,...)	do{ if(!(x)){VULCANITE_CORE_ERROR("Assertion Failed: {0}",__VA_ARGS__);__debugbreak();}}while(0)
#else
	#define VULCANITE_ASSERT(x,...)			do{if(!(x)){sizeof(__VA_ARGS__);}}while(0)
	#define VULCANITE_CORE_ASSERT(x,...)	do{if(!(x)){sizeof(__VA_ARGS__);}}while(0)
#endif