#pragma once

#include <string>

#include "Core/Base.h"

#include "Renderer/BufferLayout.h"

namespace Vulcanite {
	enum class PrimitiveTopology{
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

	struct PipelineSpe {
		std::string VertexShaderPath;
		std::string FragmentShaderPath;

		BufferLayout Layout;

		PrimitiveTopology Topology = PrimitiveTopology::TriangleList;

		CullMode    Cull = CullMode::Back;
		FrontFace   Face = FrontFace::CounterClockwise;
		PolygonMode Polygon = PolygonMode::Fill;
		BlendMode   Blend = BlendMode::None;

		bool DepthTest = false;
		bool DepthWrite = false;
		uint32_t SampleCount = 1;      // 必须与 renderPass 的附件采样数一致(当前是单采样)
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