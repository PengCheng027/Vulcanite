#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Core/Base.h"

#include "Renderer/BufferLayout.h"

namespace Vulcanite {
	enum class PrimitiveTopology {
		TriangleList, 
		LineList, 
		PointList
	};

	enum class CullMode { 
		None, 
		Front, 
		Back, 
		FrontAndBack 
	};

	enum class FrontFace { 
		CounterClockwise, 
		Clockwise 
	};

	enum class PolygonMode { 
		Fill,
		Line, 
		Point 
	};

	enum class BlendMode { 
		None, 
		Alpha 
	};

	// ===== 着色器阶段(位标记)=====
	enum class ShaderStage : uint32_t {
		None = 0,
		Vertex = 1u << 0,
		Fragment = 1u << 1,
	};

	inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
		return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline ShaderStage& operator|=(ShaderStage& a, ShaderStage b) {
		a = a | b;
		return a;
	}
	inline bool HasShaderStage(ShaderStage value, ShaderStage test) {
		return (static_cast<uint32_t>(value) & static_cast<uint32_t>(test)) != 0;
	}

	// ===== 描述符绑定(平台无关)=====
	enum class DescriptorType {
		UniformBuffer,
		Texture2D,        // 组合图像采样器
	};

	struct DescriptorBinding {
		uint32_t Binding = 0;
		DescriptorType Type = DescriptorType::UniformBuffer;
		ShaderStage Stages = ShaderStage::Vertex | ShaderStage::Fragment;
		uint32_t Count = 1;
	};

	struct PipelineSpe {
		std::string VertexShaderPath;
		std::string FragmentShaderPath;

		// 顶点布局:决定 VkVertexInput*,也决定"一条管线只接受哪种顶点数据"
		BufferLayout Layout;

		// 描述符绑定;留空 = 使用 DefaultFrameBindings()(binding 0 的相机 UBO)
		std::vector<DescriptorBinding> Bindings;

		PrimitiveTopology Topology = PrimitiveTopology::TriangleList;

		CullMode    Cull = CullMode::Back;
		FrontFace   Face = FrontFace::CounterClockwise;
		PolygonMode Polygon = PolygonMode::Fill;
		BlendMode   Blend = BlendMode::None;

		bool DepthTest = false;
		bool DepthWrite = false;
		uint32_t SampleCount = 1;      // 必须与 renderPass 的附件采样数一致(当前是单采样)

		// 帧级默认绑定:binding 0 = 相机 UBO,顶点 + 片元阶段都能读
		// 注意:VulkanContext 创建"帧描述符集"时用的是同一份定义,
		//      两边保持一致才能共用同一份描述符集。
		static std::vector<DescriptorBinding> DefaultFrameBindings() {
			return {
				DescriptorBinding{ 0, DescriptorType::UniformBuffer,
					ShaderStage::Vertex | ShaderStage::Fragment, 1 },
			};
		}

		// 实际生效的绑定列表(空 → 默认)
		std::vector<DescriptorBinding> GetBindings() const {
			return Bindings.empty() ? DefaultFrameBindings() : Bindings;
		}
	};

	class Pipeline {
	public:
		virtual ~Pipeline() = default;

		virtual void Bind() const = 0;
		virtual bool IsValid() const = 0;
		virtual const PipelineSpe& GetSpec() const = 0;

		// 工厂:具体后端在 Platform 层实现,调用方拿到的永远是 Ref<Pipeline>
		static Ref<Pipeline> CreatePipeline(const PipelineSpe& spec);
	};
}