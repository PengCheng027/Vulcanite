#include "Core/Assert.h"

#include "Renderer/Material.h"
#include "Renderer/RendererAPI.h"

#include "Platform/Vulkan/VulkanMaterial.h"

namespace Vulcanite {
	Ref<Material> Material::Create(const std::string& name) {
		switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::None: VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanMaterial>();
			case RendererAPI::API::D3D:	VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}

		VULCANITE_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}