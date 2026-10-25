#version 450

// 没有顶点颜色输入,直接输出固定颜色(橙色),便于和彩虹网格区分。

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0, 0.45, 0.1, 1.0);
}
