#include "Core/Assert.h"

#include "Renderer/BufferLayout.h"

namespace Vulcanite {
	uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
			case ShaderDataType::None:		break;
		}

		VULCANITE_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	uint32_t ShaderDataTypeComponentCount(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:		return 1;
			case ShaderDataType::Float2:	return 2;
			case ShaderDataType::Float3:	return 3;
			case ShaderDataType::Float4:	return 4;
			case ShaderDataType::Mat3:		return 9;
			case ShaderDataType::Mat4:		return 16;
			case ShaderDataType::Int:		return 1;
			case ShaderDataType::Int2:		return 2;
			case ShaderDataType::Int3:		return 3;
			case ShaderDataType::Int4:		return 4;
			case ShaderDataType::Bool:		return 1;
			case ShaderDataType::None:		break;
		}

		VULCANITE_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}
	
	uint32_t VertexAttribute::Size() {
		return ShaderDataTypeSize(Type);
	}

	uint32_t VertexAttribute::ComponentCount() {
		return ShaderDataTypeComponentCount(Type);
	}

	BufferLayout::BufferLayout(const std::initializer_list<VertexAttribute>& attributes)
		:m_Attributes(attributes) {
		CalculateOffsetsAndStride();
	}
	

	uint32_t BufferLayout::GetStride() const {
		return m_Stride;
	}

	const std::vector<VertexAttribute>& BufferLayout::GetAttributes() const {
		return m_Attributes;
	}

	std::vector<VertexAttribute>::const_iterator BufferLayout::begin() const {
		return m_Attributes.begin();
	}

	std::vector<VertexAttribute>::const_iterator BufferLayout::end() const {
		return m_Attributes.end();
	}

	int BufferLayout::GetAttributeIndex(const std::string& name) const {
		for (size_t i = 0; i < m_Attributes.size(); i++) {
			if (m_Attributes[i].Name == name) {
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	void BufferLayout::CalculateOffsetsAndStride() {
		uint32_t offset = 0;
		m_Stride = 0;
		for (VertexAttribute& attribute : m_Attributes) {
			VULCANITE_CORE_ASSERT(attribute.Size() > 0, "ShaderDataType::None 不能作为顶点属性!");
			attribute.Offset = offset;
			offset += attribute.Size();
			m_Stride += attribute.Size();
		}
	}
}