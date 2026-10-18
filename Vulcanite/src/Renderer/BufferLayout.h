#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Vulcanite {
	enum class ShaderDataType {
		None = 0,
		Float, Float2, Float3, Float4,
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool
	};

	uint32_t ShaderDataTypeSize(ShaderDataType type);
	uint32_t ShaderDataTypeComponentCount(ShaderDataType type);

	struct VertexAttribute {
		std::string Name;
		ShaderDataType Type;
		uint32_t Offset;
		bool Normalized;

		VertexAttribute(ShaderDataType type, const std::string& name, bool normalized = false)
			:Name(name), Type(type), Offset(0), Normalized(normalized) {
		}

		uint32_t Size();
		uint32_t ComponentCount();
	};

	class BufferLayout {
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<VertexAttribute>& attributes);

		uint32_t GetStride() const;
		const std::vector<VertexAttribute>& GetAttributes() const;

		std::vector<VertexAttribute>::const_iterator begin() const;
		std::vector<VertexAttribute>::const_iterator end() const;

		int GetAttributeIndex(const std::string& name) const;
	private:
		void CalculateOffsetsAndStride();
		std::vector<VertexAttribute> m_Attributes;
		uint32_t m_Stride = 0;
	};
}