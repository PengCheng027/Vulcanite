#include "Core/Assert.h"

#include "Renderer/RendererAPI.h"
#include "Renderer/Mesh.h"

#include "Platform/Vulkan/VulkanMesh.h"

namespace Vulcanite {
	Ref<Mesh> Mesh::Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::None:	VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanMesh>(vertices, indices);
			case RendererAPI::API::D3D:	VULCANITE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		}

		VULCANITE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}