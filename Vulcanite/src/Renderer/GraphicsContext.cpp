#include "Core/Assert.h"

#include "Renderer/GraphicsContext.h"
#include "Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Vulcanite {
	Scope<GraphicsContext> GraphicsContext::Create(void* windowHandle) {
		switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::None: VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateScope<VulkanContext>();
			case RendererAPI::API::D3D: VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}
		
		VULCANITE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
		
}