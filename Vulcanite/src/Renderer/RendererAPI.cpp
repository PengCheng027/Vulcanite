#include "Core/Assert.h"

#include "Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanRendererAPI.h"

namespace Vulcanite {
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

	Scope<RendererAPI> RendererAPI::Create() {
		switch (s_API) {
			case Vulcanite::RendererAPI::API::None: VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		
			case Vulcanite::RendererAPI::API::Vulkan: return CreateScope<VulkanRendererAPI>();
			case Vulcanite::RendererAPI::API::D3D: VULCANITE_CORE_ASSERT(false, "RendererAPI::D3D is currently not supported!"); return nullptr;
		}

		VULCANITE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	RendererAPI::API RendererAPI::GetAPI() {
		return s_API;
	}
}