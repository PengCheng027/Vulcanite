#include <cmath>

#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/RenderCommand.h"

#include "Core/Assert.h"
#include "Core/VulLog.h"
#include "Renderer/Mesh.h"
#include "Renderer/Pipeline.h"

namespace Vulcanite {

	// 一条批:自己的几何存储 + 自己的管线
	struct Renderer2DBatch {
		Ref<Mesh> BatchMesh = nullptr;
		Ref<Pipeline> Pipeline = nullptr;
	};

	struct Renderer2DData {
		static const uint32_t MaxQuads = 20000;

		// 四边形批:每 quad 4 顶点 6 索引
		static const uint32_t MaxQuadVertices = MaxQuads * 4;
		static const uint32_t MaxQuadIndices = MaxQuads * 6;

		// 折线批:每个点 2 顶点,每段 6 索引(每段两个三角形)
		static const uint32_t MaxLinePoints = MaxQuads;
		static const uint32_t MaxLineVertices = MaxLinePoints * 2;
		static const uint32_t MaxLineIndices = MaxLinePoints * 6;

		glm::vec4 QuadVertexPositions[4] = {};

		// 两条独立的批:数据直接存在各自的 Mesh 里
		Renderer2DBatch QuadBatch;
		Renderer2DBatch LineBatch;

		// 默认管线(两条批初始都用它;之后可以各自换成别的)
		Ref<Pipeline> pDefaultPipeline = nullptr;
	};

	static Renderer2DData s_Data;

	// ===== 批内部辅助(文件内静态函数,不进头文件)=====

	static bool EnsureCapacity(Renderer2DBatch& batch, uint32_t vertexCount, uint32_t indexCount) {
		if (batch.BatchMesh == nullptr) {
			return false;
		}

		const uint32_t vertexCapacity = batch.BatchMesh->GetVertexCapacity();
		const uint32_t indexCapacity = batch.BatchMesh->GetIndexCapacity();

		if (vertexCount > vertexCapacity || indexCount > indexCapacity) {
			VULCANITE_CORE_ERROR("单次绘制超过整批容量:顶点 {0}/{1},索引 {2}/{3}",
				vertexCount, vertexCapacity, indexCount, indexCapacity);
			return false;
		}

		if (batch.BatchMesh->GetVertexCount() + vertexCount > vertexCapacity ||
			batch.BatchMesh->GetIndexCount() + indexCount > indexCapacity) {
			// 当前批放不下了:先结算这一条批(另一条批不受影响),
			// 返回后调用方会从空批继续 AppendVertex/AppendIndices。
			if (batch.BatchMesh->GetVertexCount() > 0 && batch.BatchMesh->GetIndexCount() > 0) {
				RenderCommand::DrawIndex(*batch.BatchMesh, batch.Pipeline);
				batch.BatchMesh->ResetGeometry();
			}
		}
		return true;
	}

	static void EmitVertex(Renderer2DBatch& batch, const glm::vec2& position, const glm::vec4& color) {
		batch.BatchMesh->AppendVertex({ position, glm::vec3(color) });
	}

	static void EmitTriangle(Renderer2DBatch& batch, uint32_t a, uint32_t b, uint32_t c) {
		const uint32_t indices[3] = { a, b, c };
		batch.BatchMesh->AppendIndices(indices, 3);
	}

	// 线段拓扑:每两个索引一段线(成对消费)
	static void EmitLine(Renderer2DBatch& batch, uint32_t a, uint32_t b) {
		const uint32_t indices[2] = { a, b };
		batch.BatchMesh->AppendIndices(indices, 2);
	}

	static void FlushBatch(Renderer2DBatch& batch) {
		if (batch.BatchMesh == nullptr) {
			return;
		}
		if (batch.BatchMesh->GetVertexCount() == 0 || batch.BatchMesh->GetIndexCount() == 0) {
			return;   // 这条批没有几何,不生成命令
		}

		// 几何已经在网格里了:直接提交这一批,再归零复用
		RenderCommand::DrawIndex(*batch.BatchMesh, batch.Pipeline);
		batch.BatchMesh->ResetGeometry();
	}

	// ===== 公开接口 =====

	void Renderer2D::Init() {
		// 两条批各自一次性分配容量,之后每帧复用、不再分配
		s_Data.QuadBatch.BatchMesh = Mesh::Create(
			std::vector<Vertex>(Renderer2DData::MaxQuadVertices),
			std::vector<uint32_t>(Renderer2DData::MaxQuadIndices));
		s_Data.QuadBatch.BatchMesh->ResetGeometry();

		s_Data.LineBatch.BatchMesh = Mesh::Create(
			std::vector<Vertex>(Renderer2DData::MaxLineVertices),
			std::vector<uint32_t>(Renderer2DData::MaxLineIndices));
		s_Data.LineBatch.BatchMesh->ResetGeometry();

		s_Data.QuadVertexPositions[0] = { -0.5f,-0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = {  0.5f,-0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = {  0.5f, 0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f, 0.5f, 0.0f, 1.0f };

		// 默认管线由渲染器自己创建:自己的着色器 + 自己的顶点布局 + 帧级默认 UBO
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

		// 两条批默认共用同一条管线(同一个对象,不重复创建 VkPipeline)
		s_Data.QuadBatch.Pipeline = s_Data.pDefaultPipeline;
		s_Data.LineBatch.Pipeline = s_Data.pDefaultPipeline;
	}

	void Renderer2D::ShutDown() {
		s_Data.QuadBatch = Renderer2DBatch{};
		s_Data.LineBatch = Renderer2DBatch{};
		s_Data.pDefaultPipeline = nullptr;
	}

	void Renderer2D::BeginSence(const glm::mat4& viewProjectMatrix) {
		RenderCommand::Begin(viewProjectMatrix);

		// 清掉上一帧可能残留的未结算几何;容量保留
		if (s_Data.QuadBatch.BatchMesh != nullptr) {
			s_Data.QuadBatch.BatchMesh->ResetGeometry();
		}
		if (s_Data.LineBatch.BatchMesh != nullptr) {
			s_Data.LineBatch.BatchMesh->ResetGeometry();
		}

		// 每帧从默认管线重新开始,避免上一帧切换后的管线"泄漏"到下一帧
		s_Data.QuadBatch.Pipeline = s_Data.pDefaultPipeline;
		s_Data.LineBatch.Pipeline = s_Data.pDefaultPipeline;
	}

	void Renderer2D::EndSence() {
		Flush();
	}

	void Renderer2D::SetQuadPipeline(const Ref<Pipeline>& pipeline) {
		if (pipeline == nullptr || pipeline == s_Data.QuadBatch.Pipeline) {
			return;
		}

		FlushBatch(s_Data.QuadBatch);   // 只结算四边形批
		s_Data.QuadBatch.Pipeline = pipeline;
	}

	void Renderer2D::SetLinePipeline(const Ref<Pipeline>& pipeline) {
		if (pipeline == nullptr || pipeline == s_Data.LineBatch.Pipeline) {
			return;
		}

		FlushBatch(s_Data.LineBatch);   // 只结算折线批
		s_Data.LineBatch.Pipeline = pipeline;
	}

	Ref<Pipeline> Renderer2D::GetQuadPipeline() {
		return s_Data.QuadBatch.Pipeline;
	}

	Ref<Pipeline> Renderer2D::GetLinePipeline() {
		return s_Data.LineBatch.Pipeline;
	}

	void Renderer2D::Flush() {
		// 先四边形、后折线:同一帧内的绘制顺序稳定(折线盖在四边形上面)
		FlushBatch(s_Data.QuadBatch);
		FlushBatch(s_Data.LineBatch);
	}

	// ===== 图元 =====

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec4& color, const glm::mat4& transform) {
		Renderer2DBatch& batch = s_Data.QuadBatch;
		if (!EnsureCapacity(batch, 4, 6)) {
			return;
		}

		// base 必须在 EnsureCapacity 之后取(它内部可能结算并把计数归零)
		const uint32_t base = batch.BatchMesh->GetVertexCount();

		// CPU 只输出世界坐标:应用物体变换 + 平移,不做投影(投影由 shader 用 UBO 完成)
		for (int i = 0; i < 4; i++) {
			glm::vec4 worldPos = transform * s_Data.QuadVertexPositions[i];
			worldPos += glm::vec4(position, 0.0f);
			EmitVertex(batch, glm::vec2(worldPos), color);
		}

		EmitTriangle(batch, base + 0, base + 1, base + 2);
		EmitTriangle(batch, base + 2, base + 3, base + 0);
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, float width, const glm::vec4& color) {
		const glm::vec3 points[2] = { p0, p1 };
		DrawPolyline(points, 2, width, color, false);
	}

	void Renderer2D::DrawPolyline(const glm::vec3* points, uint32_t pointCount, float width,
		const glm::vec4& color, bool closed) {
		if (points == nullptr || pointCount < 2) {
			return;
		}

		Renderer2DBatch& batch = s_Data.LineBatch;
		const uint32_t segmentCount = closed ? pointCount : pointCount - 1;

		// 几何形状由"折线批当前管线的拓扑"决定:
		//   LineList     → 每段只提交两个端点(硬件细线,线宽固定 1px,width 被忽略)
		//   TriangleList → CPU 展开成有宽度的三角形条带(粗线,可控宽度)
		const bool lineTopology = batch.Pipeline != nullptr &&
			batch.Pipeline->GetSpec().Topology == PrimitiveTopology::LineList;

		if (lineTopology) {
			const uint32_t vertexCount = segmentCount * 2;    // 每段:起点 + 终点
			const uint32_t indexCount = segmentCount * 2;     // LINE_LIST 成对消费

			if (!EnsureCapacity(batch, vertexCount, indexCount)) {
				return;
			}

			const uint32_t base = batch.BatchMesh->GetVertexCount();

			for (uint32_t s = 0; s < segmentCount; ++s) {
				const uint32_t i0 = s;
				const uint32_t i1 = (s + 1) % pointCount;     // closed 时最后一段回到首点
				EmitVertex(batch, glm::vec2(points[i0]), color);
				EmitVertex(batch, glm::vec2(points[i1]), color);
			}

			for (uint32_t s = 0; s < segmentCount; ++s) {
				const uint32_t a = base + 2 * s;
				EmitLine(batch, a, a + 1);
			}
			return;
		}

		if (width <= 0.0f) {
			return;
		}

		const uint32_t vertexCount = pointCount * 2;          // 每个点:左 / 右 两个顶点
		const uint32_t indexCount = segmentCount * 6;         // 每段:两个三角形

		if (!EnsureCapacity(batch, vertexCount, indexCount)) {
			return;
		}

		const uint32_t base = batch.BatchMesh->GetVertexCount();
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

			EmitVertex(batch, p + normal, color);   // 左
			EmitVertex(batch, p - normal, color);   // 右
		}

		// 每段两个三角形;顶点顺序与 quad 一致(世界坐标下逆时针),避免被背面剔除
		for (uint32_t s = 0; s < segmentCount; ++s) {
			const uint32_t left0 = base + 2 * s;
			const uint32_t right0 = left0 + 1;
			const uint32_t nextPoint = (s + 1) % pointCount;
			const uint32_t left1 = base + 2 * nextPoint;
			const uint32_t right1 = left1 + 1;

			EmitTriangle(batch, left0, right0, right1);
			EmitTriangle(batch, left0, right1, left1);
		}
	}
}
