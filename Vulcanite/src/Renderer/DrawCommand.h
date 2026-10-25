#pragma once

#include <cstdint>

#include "Core/Base.h"

#include "Renderer/Pipeline.h"

namespace Vulcanite {
	/**
	 * 一条绘制命令 = 「画哪一段几何」+「用哪条管线」。
	 *
	 * 这是"绘制命令列表"架构的最小数据单元:上层(Renderer2D / 调用方)负责把几何
	 * 写进本帧的动态缓冲并生成命令,渲染上下文只负责逐条执行 —— 于是"不同网格用
	 * 不同管线"变成"命令里带着不同的 Pipeline",不需要任何全局的"当前管线"。
	 *
	 * 几何位置用**字节偏移**表示(而不是"顶点序号"):同一条缓冲里因此可以容纳
	 * 不同顶点布局(不同 stride)的几何 —— 每条命令绑定时把顶点缓冲偏移到自己的
	 * 起点,索引通过 FirstIndex 指向索引缓冲。
	 */
	struct DrawCommand {
		Ref<Pipeline> Pipeline = nullptr;   // 用哪条管线(为空则跳过)
		uint32_t IndexCount = 0;            // 画多少个索引
		uint32_t FirstIndex = 0;            // 在本帧索引缓冲中的起始索引(单位:个 uint32)
		uint32_t VertexByteOffset = 0;      // 顶点缓冲中的字节偏移(适配不同 stride 的布局)
	};
}
