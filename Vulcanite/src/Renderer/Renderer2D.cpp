#include <cmath>

#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"

#include "Core/Assert.h"
#include "Core/VulLog.h"
#include "Renderer/Mesh.h"
#include "Renderer/Pipeline.h"

namespace Vulcanite {

	struct Renderer2DData {
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		// 批的顶点/索引数据**直接存在 pMesh 里**(容量在 Init 时一次分配)。
		// Renderer2D 不再单独维护 CPU 数组,三件事都交给 Mesh:
		//   收集 → Mesh::AppendVertex / AppendIndices
		//   计数 → Mesh::GetVertexCount / GetIndexCount
		//   结算 → DrawIndex(直接读网格)+ Mesh::ResetGeometry
		glm::vec4 QuadVertexPositions[4] = {};

		Ref<Mesh> pMesh = nullptr;

		// 当前批使用的管线:批内只用一条,换管线时先结算
		Ref<Pipeline> pPipeline = nullptr;

		// 每帧开始时的默认管线(帧首复位用,避免上一帧切换后的管线"泄漏"到下一帧)
		Ref<Pipeline> pDefaultPipeline = nullptr;
	};

	static Renderer2DData s_Data;

	void Renderer2D::Init() {
		// 顶点/索引的容量在这里一次性分配,之后每帧只复用、不再分配
		s_Data.pMesh = Mesh::Create(std::vector<Vertex>(Renderer2DData::MaxVertices),
			std::vector<uint32_t>(Renderer2DData::MaxIndices));
		s_Data.pMesh->ResetGeometry();

		s_Data.QuadVertexPositions[0] = { -0.5f,-0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f,-0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f, 0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

		// 默认管线由渲染器自己创建(不再由 VulkanContext 代劳):
		// 规格 = 自己的着色器 + 自己的顶点布局 + 描述符绑定(留空 = 帧级默认 UBO)
		PipelineSpe spec;
		spec.VertexShaderPath = "assets/spir_v/shader_base.vert.spv";
		spec.FragmentShaderPath = "assets/spir_v/shader_base.frag.spv";
		spec.Layout = BufferLayout{
			VertexAttribute(ShaderDataType::Float2,"m_Pos"),
			VertexAttribute(ShaderDataType::Float3,"m_Color")
		};
		spec.Topology = PrimitiveTopology::TriangleList;
		spec.Cull = CullMode::Back;
		spec.Face = FrontFace::CounterClockwise;
		spec.Polygon = PolygonMode::Fill;
		spec.Blend = BlendMode::None;
		spec.SampleCount = 1;

		s_Data.pDefaultPipeline = Pipeline::CreatePipeline(spec);
		s_Data.pPipeline = s_Data.pDefaultPipeline;
	}

	void Renderer2D::ShutDown() {
		s_Data.pMesh = nullptr;
		s_Data.pPipeline = nullptr;
		s_Data.pDefaultPipeline = nullptr;
	}

	void Renderer2D::BeginSence(const glm::mat4& viewProjectMatrix) {
		RenderCommand::Begin(viewProjectMatrix);

		if (s_Data.pMesh != nullptr) {
			// 清掉上一帧可能残留的未结算几何;容量保留
			s_Data.pMesh->ResetGeometry();
		}

		// 关键:每帧从默认管线重新开始。否则上一帧 SetPipeline 切换后的管线会残留,
		// 导致下一帧整帧都用那条管线(批的划分也随之消失)。
		s_Data.pPipeline = s_Data.pDefaultPipeline;
	}

	void Renderer2D::EndSence() {
		Flush();
	}

	void Renderer2D::SetPipeline(const Ref<Pipeline>& pipeline) {
		if (pipeline == nullptr || pipeline == s_Data.pPipeline) {
			return;
		}

		Flush();                       // 换管线前先把当前批结算掉(批内只能有一条管线)
		s_Data.pPipeline = pipeline;
	}

	Ref<Pipeline> Renderer2D::GetPipeline() {
		return s_Data.pPipeline;
	}

	bool Renderer2D::EnsureCapacity(uint32_t vertexCount, uint32_t indexCount) {
		if (s_Data.pMesh == nullptr) {
			return false;
		}

		const uint32_t vertexCapacity = s_Data.pMesh->GetVertexCapacity();
		const uint32_t indexCapacity = s_Data.pMesh->GetIndexCapacity();

		if (vertexCount > vertexCapacity || indexCount > indexCapacity) {
			VULCANITE_CORE_ERROR("单次绘制超过整批容量:顶点 {0}/{1},索引 {2}/{3}",
				vertexCount, vertexCapacity, indexCount, indexCapacity);
			return false;
		}

		if (s_Data.pMesh->GetVertexCount() + vertexCount > vertexCapacity ||
			s_Data.pMesh->GetIndexCount() + indexCount > indexCapacity) {
			// 当前批放不下了:把已收集的几何结算成一条绘制命令,并把计数归零 ——
			// 返回后调用方会从空批继续 AppendVertex/AppendIndices。
			// (这里不写任何顶点/索引,写入只发生在调用方。)
			Flush();
		}
		return true;
	}

	void Renderer2D::EmitVertex(const glm::vec2& position, const glm::vec4& color) {
		s_Data.pMesh->AppendVertex({ position, glm::vec3(color) });
	}

	void Renderer2D::EmitTriangle(uint32_t a, uint32_t b, uint32_t c) {
		const uint32_t indices[3] = { a, b, c };
		s_Data.pMesh->AppendIndices(indices, 3);
	}

	void Renderer2D::Flush() {
		if (s_Data.pMesh == nullptr) {
			return;
		}
		if (s_Data.pMesh->GetVertexCount() == 0 || s_Data.pMesh->GetIndexCount() == 0) {
			return;   // 本批没有几何,不生成命令
		}

		// 几何已经在网格里了:不需要再拷贝/上传,直接提交这一批
		RenderCommand::DrawIndex(*s_Data.pMesh, s_Data.pPipeline);

		// 结算后计数归零,容量保留,下一批继续复用
		s_Data.pMesh->ResetGeometry();
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec4& color, const glm::mat4& transform) {
		if (!EnsureCapacity(4, 6)) {
			return;
		}

		// 注意:base 必须在 EnsureCapacity 之后取(它内部可能 Flush 并把计数归零)
		const uint32_t base = s_Data.pMesh->GetVertexCount();

		// CPU 只输出世界坐标:应用物体变换 + 平移,不做投影(投影由 shader 用 UBO 完成)
		for (int i = 0; i < 4; i++) {
			glm::vec4 worldPos = transform * s_Data.QuadVertexPositions[i];
			worldPos += glm::vec4(position, 0.0f);
			EmitVertex(glm::vec2(worldPos), color);
		}

		EmitTriangle(base + 0, base + 1, base + 2);
		EmitTriangle(base + 2, base + 3, base + 0);
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, float width, const glm::vec4& color) {
		const glm::vec3 points[2] = { p0, p1 };
		DrawPolyline(points, 2, width, color, false);
	}

	void Renderer2D::DrawPolyline(const glm::vec3* points, uint32_t pointCount, float width,
		const glm::vec4& color, bool closed) {
		if (points == nullptr || pointCount < 2 || width <= 0.0f) {
			return;
		}

		const uint32_t segmentCount = closed ? pointCount : pointCount - 1;
		const uint32_t vertexCount = pointCount * 2;          // 每个点:左 / 右 两个顶点
		const uint32_t indexCount = segmentCount * 6;         // 每段:两个三角形

		if (!EnsureCapacity(vertexCount, indexCount)) {
			return;
		}

		const uint32_t base = s_Data.pMesh->GetVertexCount();
		const float halfWidth = width * 0.5f;

		for (uint32_t i = 0; i < pointCount; ++i) {
			const glm::vec2 p(points[i]);

			// 切线用前后两点差分(端点用单侧;闭合时首尾相接),得到"平均法线"式的连接
			glm::vec2 prev, next;
			if (closed) {
				prev = glm::vec2(points[(i + pointCount - 1) % pointCount]);
				next = glm::vec2(points[(i + 1) % pointCount]);
			}
			else {
				prev = glm::vec2(points[i == 0 ? 0 : i - 1]);
				next = glm::vec2(points[i + 1 >= pointCount ? pointCount - 1 : i + 1]);
			}

			glm::vec2 tangent = next - prev;
			if (glm::dot(tangent, tangent) < 1e-12f) {
				tangent = glm::vec2(1.0f, 0.0f);   // 重复点:退化为水平方向
			}

			const glm::vec2 normal = glm::normalize(glm::vec2(-tangent.y, tangent.x)) * halfWidth;

			EmitVertex(p + normal, color);   // 左
			EmitVertex(p - normal, color);   // 右
		}

		// 每段两个三角形;顶点顺序与 quad 一致(世界坐标下逆时针),避免被背面剔除
		for (uint32_t s = 0; s < segmentCount; ++s) {
			const uint32_t left0 = base + 2 * s;
			const uint32_t right0 = left0 + 1;
			const uint32_t nextPoint = (s + 1) % pointCount;
			const uint32_t left1 = base + 2 * nextPoint;
			const uint32_t right1 = left1 + 1;

			EmitTriangle(left0, right0, right1);
			EmitTriangle(left0, right1, left1);
		}
	}
}
