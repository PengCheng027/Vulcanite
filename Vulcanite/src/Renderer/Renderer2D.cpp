#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"

#include "Renderer/Mesh.h"

namespace Vulcanite {

	struct Renderer2DData {
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxIndices = MaxQuads * 6;

		Vertex* QuadVertexBufferBase = nullptr;
		Vertex* QuadVertexBufferPtr = nullptr;

		float aspect = 1280.0f / 720.0f;

		// bottom=1, top=-1 → 正交矩阵自带 Y 翻转(适配 Vulkan 的 NDC Y 轴向下)
		// near/far 用 0.1~100 是为了覆盖 lookAt 把物体推到相机空间 z=-2 的深度
		glm::mat4 proj = glm::ortho(-aspect, aspect, 1.0f, -1.0f, 0.1f, 100.0f);

		glm::mat4 ViewProject = proj *
			glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec4 QuadVertexPositions[4] = {};

		Ref<Mesh> pMesh = nullptr;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init() {
		s_Data.pMesh = Mesh::Create(std::vector<Vertex>(s_Data.MaxQuads * 4), std::vector<uint32_t>(s_Data.MaxIndices));

		s_Data.QuadVertexBufferBase = new Vertex[s_Data.MaxQuads * 4];

		uint32_t* quadIndexes = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
			quadIndexes[i + 0] = 0 + offset;
			quadIndexes[i + 1] = 1 + offset;
			quadIndexes[i + 2] = 2 + offset;

			quadIndexes[i + 3] = 2 + offset;
			quadIndexes[i + 4] = 3 + offset;
			quadIndexes[i + 5] = 0 + offset;

			offset += 4;
		}

		s_Data.pMesh->SetIndexDatas(quadIndexes, s_Data.MaxIndices);
		
		delete[] quadIndexes;

		s_Data.QuadVertexPositions[0] = { -0.5f,-0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f,-0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f, 0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };
	}

	void Renderer2D::ShutDown() {
		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::BeginSence(const glm::mat4& viewProjectMatrix) {
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::EndSence() {
		uint32_t vertextCount = static_cast<uint32_t>(s_Data.QuadVertexBufferPtr - s_Data.QuadVertexBufferBase);
		s_Data.pMesh->SetVertexDatas(s_Data.QuadVertexBufferBase, vertextCount);
		s_Data.pMesh->SetVertexCount(vertextCount);

		uint32_t indexCount = vertextCount / 4 * 6;

		s_Data.pMesh->SetIndexCount(indexCount);

		RenderCommand::DrawIndex(*s_Data.pMesh);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec4& color, const glm::mat4& transform) {
		// CPU 只输出世界坐标:应用物体变换 + 平移,不做投影(投影由 shader 用 UBO 完成)
		for (int i = 0; i < 4; i++) {
			glm::vec4 worldPos = transform * s_Data.QuadVertexPositions[i];
			worldPos += glm::vec4(position, 0.0f);

			s_Data.QuadVertexBufferPtr[i] = { glm::vec2(worldPos), glm::vec3(color) };
		}

		s_Data.QuadVertexBufferPtr += 4;
	}
}