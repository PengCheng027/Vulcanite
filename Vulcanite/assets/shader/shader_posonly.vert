#version 450

// 演示"不同顶点布局":这个顶点着色器只接收位置(没有颜色属性),
// 对应的 PipelineSpe::Layout 只有一项 Float2,stride = 8 字节(而不是 20)。

layout(binding = 0) uniform uniformBufferObject {
	mat4 viewProject;
} ubo;

layout(location = 0) in vec2 inPosition;

void main() {
	gl_Position = ubo.viewProject * vec4(inPosition, 0.0, 1.0);
}
