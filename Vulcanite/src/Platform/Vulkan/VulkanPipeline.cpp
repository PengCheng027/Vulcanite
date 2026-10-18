#include "Core/Assert.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanPipeline.h"

#include "Utils/FileUtils.h"

// ============================================================================
//  PipelineSpe → Vulkan 枚举/格式 的翻译,全部集中在本文件的匿名命名空间里。
//  内部链接:别的 .cpp 不该再出现这些 switch,也不会有符号冲突。
// ============================================================================
namespace {

	VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code) {
		VkShaderModule shaderModule = VK_NULL_HANDLE;

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		VULCANITE_CORE_ASSERT(result == VK_SUCCESS, "failed to create shader module!");
		return result == VK_SUCCESS ? shaderModule : VK_NULL_HANDLE;
	}

	VkFormat ToVkFormat(Vulcanite::ShaderDataType type) {
		switch (type) {
			case Vulcanite::ShaderDataType::Float:	return	VK_FORMAT_R32_SFLOAT;
			case Vulcanite::ShaderDataType::Float2:	return	VK_FORMAT_R32G32_SFLOAT;
			case Vulcanite::ShaderDataType::Float3:	return	VK_FORMAT_R32G32B32_SFLOAT;
			case Vulcanite::ShaderDataType::Float4:	return	VK_FORMAT_R32G32B32A32_SFLOAT;
			case Vulcanite::ShaderDataType::Int:	return	VK_FORMAT_R32_SINT;
			case Vulcanite::ShaderDataType::Int2:	return	VK_FORMAT_R32G32_SINT;
			case Vulcanite::ShaderDataType::Int3:	return	VK_FORMAT_R32G32B32_SINT;
			case Vulcanite::ShaderDataType::Int4:	return	VK_FORMAT_R32G32B32A32_SINT;
			// Mat3/Mat4 需要按列拆成多个 location,Bool 没有对应的顶点格式
			default:								return	VK_FORMAT_UNDEFINED;
		}
	}

	VkPrimitiveTopology ToVkTopology(Vulcanite::PrimitiveTopology topology) {
		switch (topology) {
			case Vulcanite::PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case Vulcanite::PrimitiveTopology::LineList:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case Vulcanite::PrimitiveTopology::PointList:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}

		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	VkCullModeFlags ToVkCullMode(Vulcanite::CullMode mode) {
		switch (mode) {
			case Vulcanite::CullMode::None:         return VK_CULL_MODE_NONE;
			case Vulcanite::CullMode::Front:        return VK_CULL_MODE_FRONT_BIT;
			case Vulcanite::CullMode::Back:         return VK_CULL_MODE_BACK_BIT;
			case Vulcanite::CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
		}
		return VK_CULL_MODE_NONE;
	}

	VkFrontFace ToVkFrontFace(Vulcanite::FrontFace face) {
		return face == Vulcanite::FrontFace::Clockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	VkPolygonMode ToVkPolygonMode(Vulcanite::PolygonMode mode) {
		switch (mode) {
			case Vulcanite::PolygonMode::Fill:  return VK_POLYGON_MODE_FILL;
			case Vulcanite::PolygonMode::Line:  return VK_POLYGON_MODE_LINE;
			case Vulcanite::PolygonMode::Point: return VK_POLYGON_MODE_POINT;
		}
		return VK_POLYGON_MODE_FILL;
	}

	VkSampleCountFlagBits ToVkSampleCount(uint32_t samples) {
		switch (samples) {
			case 1:  return VK_SAMPLE_COUNT_1_BIT;
			case 2:  return VK_SAMPLE_COUNT_2_BIT;
			case 4:  return VK_SAMPLE_COUNT_4_BIT;
			case 8:  return VK_SAMPLE_COUNT_8_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 64: return VK_SAMPLE_COUNT_64_BIT;
		}
		VULCANITE_CORE_ASSERT(false, "不支持的采样数!");
		return VK_SAMPLE_COUNT_1_BIT;
	}

	void FillColorBlendAttachment(Vulcanite::BlendMode mode, VkPipelineColorBlendAttachmentState& out) {
		out.blendEnable = (mode == Vulcanite::BlendMode::Alpha) ? VK_TRUE : VK_FALSE;
		out.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;    // 直通 alpha:src*a + dst*(1-a)
		out.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		out.colorBlendOp = VK_BLEND_OP_ADD;
		out.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		out.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		out.alphaBlendOp = VK_BLEND_OP_ADD;
	}
}

namespace Vulcanite {
	VulkanPipeline::VulkanPipeline(const PipelineSpe& spec)
		:m_Spec(spec) {
		VulkanContext* context = VulkanContext::Get();
		// VulkanContext 析构第一步就把单例指针清空了,这里必须判空
		VULCANITE_CORE_ASSERT(context != nullptr, "VulkanContext 尚未初始化(或已析构)!");
		if (context == nullptr) {
			return;
		}

		m_Device = context->GetDevice();
		m_RenderPass = context->GetRenderPass();
		m_Subpass = 0;

		VULCANITE_CORE_ASSERT(spec.Layout.GetStride() > 0, "PipelineSpe::Layout 未设置!");
		if (spec.Layout.GetStride() == 0) {
			return;
		}

		CreateDescriptorSetLayout();
		CreatePipelineLayout();
		CreateGraphicsPipeline();
	}

	VulkanPipeline::~VulkanPipeline() {
		Destroy();
	}

	void VulkanPipeline::Bind() const {
		VULCANITE_CORE_ASSERT(m_GraphicsPipeline != VK_NULL_HANDLE, "VulkanPipeline 尚未创建成功,不能绑定!");
		if (m_GraphicsPipeline == VK_NULL_HANDLE) {
			return;
		}

		// 平台无关的 Bind() 不带参数,从上下文取"当前正在录制的命令缓冲"
		VulkanContext* context = VulkanContext::Get();
		if (context == nullptr) {
			return;
		}

		VkCommandBuffer cmd = context->GetCurrentCommandBuffer();
		VULCANITE_CORE_ASSERT(cmd != VK_NULL_HANDLE, "当前没有正在录制的命令缓冲!");
		if (cmd == VK_NULL_HANDLE) {
			return;
		}

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
	}

	bool VulkanPipeline::IsValid() const {
		return m_GraphicsPipeline != VK_NULL_HANDLE;
	}

	const PipelineSpe& VulkanPipeline::GetSpec() const {
		return m_Spec;
	}

	void VulkanPipeline::BindDescriptorSet(VkCommandBuffer cmd, VkDescriptorSet set, uint32_t firstSet) const {
		if (cmd == VK_NULL_HANDLE || m_PipelineLayout == VK_NULL_HANDLE) {
			return;
		}

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout,
			firstSet, 1, &set, 0, nullptr);
	}

	void VulkanPipeline::CreateDescriptorSetLayout() {
		// 目前只有一个描述符:binding 0 的 UBO(顶点阶段使用)
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		VkResult result = vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout);
		VULCANITE_CORE_ASSERT(result == VK_SUCCESS, "failed to create descriptor set layout!");
		if (result != VK_SUCCESS) {
			m_DescriptorSetLayout = VK_NULL_HANDLE;
		}
	}

	void VulkanPipeline::CreatePipelineLayout() {
		if (m_DescriptorSetLayout == VK_NULL_HANDLE) {
			return;
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;    // 暂时没有 push constant
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
		VULCANITE_CORE_ASSERT(result == VK_SUCCESS, "failed to create pipeline layout!");
		if (result != VK_SUCCESS) {
			m_PipelineLayout = VK_NULL_HANDLE;
		}
	}

	void VulkanPipeline::CreateGraphicsPipeline() {
		if (m_PipelineLayout == VK_NULL_HANDLE) {
			return;
		}

		// ===== 1. 着色器:读 SPIR-V → 建 module(只是容器,真正的"编译"在建管线时) =====
		std::vector<char> vertShaderCode;
		std::vector<char> fragShaderCode;
		if (!ReadFile(m_Spec.VertexShaderPath, vertShaderCode)) {
			VULCANITE_CORE_ERROR("读取顶点着色器失败:{0}", m_Spec.VertexShaderPath);
			return;                                // 早退:m_GraphicsPipeline 保持 NULL,IsValid() == false
		}
		if (!ReadFile(m_Spec.FragmentShaderPath, fragShaderCode)) {
			VULCANITE_CORE_ERROR("读取片元着色器失败:{0}", m_Spec.FragmentShaderPath);
			return;
		}

		VkShaderModule vertShaderModule = CreateShaderModule(m_Device, vertShaderCode);
		VkShaderModule fragShaderModule = CreateShaderModule(m_Device, fragShaderCode);
		if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
			if (vertShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
			if (fragShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
			return;
		}

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";        // SPIR-V 入口点名,glslc 编译 GLSL 默认就是 main

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// ===== 2. 顶点输入:由 PipelineSpe::Layout 驱动,不再依赖 VulkanMesh =====
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = m_Spec.Layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		const std::vector<VertexAttribute>& layoutAttributes = m_Spec.Layout.GetAttributes();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.reserve(layoutAttributes.size());

		for (size_t i = 0; i < layoutAttributes.size(); ++i) {
			const VertexAttribute& attribute = layoutAttributes[i];

			VkFormat format = ToVkFormat(attribute.Type);
			VULCANITE_CORE_ASSERT(format != VK_FORMAT_UNDEFINED, "顶点属性类型不能作为单个 location!");
			VULCANITE_CORE_ASSERT(!attribute.Normalized, "归一化属性需要 *_UNORM/*_SNORM 格式,当前映射表不支持");

			VkVertexInputAttributeDescription description{};
			description.binding = 0;
			description.location = static_cast<uint32_t>(i);   // location = 声明顺序,对应着色器的 layout(location = N)
			description.format = format;
			description.offset = attribute.Offset;             // 由 CalculateOffsetsAndStride 算好
			attributeDescriptions.push_back(description);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// ===== 3. 图元装配 =====
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = ToVkTopology(m_Spec.Topology);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// viewport / scissor 都是动态状态,每帧由 RecordCommandBuffer 设置
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// ===== 4. 光栅化 =====
		// 注意:PolygonMode::Line / Point 需要逻辑设备开启 VkPhysicalDeviceFeatures::fillModeNonSolid,
		// lineWidth > 1.0f 需要 wideLines。当前设备两者都没开,所以这里只用 Fill 才安全。
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = ToVkPolygonMode(m_Spec.Polygon);
		rasterizer.lineWidth = 1.0f;               // 大于 1 需要 wideLines 特性
		rasterizer.cullMode = ToVkCullMode(m_Spec.Cull);
		rasterizer.frontFace = ToVkFrontFace(m_Spec.Face);
		rasterizer.depthBiasEnable = VK_FALSE;

		// ===== 5. 多重采样:必须与 renderPass 的附件采样数一致 =====
		VULCANITE_CORE_ASSERT(m_Spec.SampleCount == 1, "当前 renderPass 是单采样,SampleCount 必须为 1!");
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = ToVkSampleCount(m_Spec.SampleCount);

		// ===== 6. 颜色混合 =====
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		FillColorBlendAttachment(m_Spec.Blend, colorBlendAttachment);   // 这里已经设过 blendEnable,后面不要再覆盖
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 1.0f;
		colorBlending.blendConstants[1] = 1.0f;
		colorBlending.blendConstants[2] = 1.0f;
		colorBlending.blendConstants[3] = 1.0f;

		// ===== 7. 动态状态:viewport/scissor 每帧设置,管线里不固化 =====
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// 当前 renderPass 没有深度附件,所以深度测试/写入必须关闭,且不提供 pDepthStencilState。
		// 将来加了深度附件,再填 VkPipelineDepthStencilStateCreateInfo 并接上 m_Spec.DepthTest/DepthWrite。
		VULCANITE_CORE_ASSERT(!m_Spec.DepthTest && !m_Spec.DepthWrite,
			"当前 renderPass 没有深度附件,DepthTest/DepthWrite 必须为 false!");

		// ===== 8. 组装并创建 =====
		// 注意:下面 pipelineInfo 里所有指针都指向本函数的局部变量,
		// 因此 vkCreateGraphicsPipelines 必须在这个作用域内调用。
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = m_RenderPass;
		pipelineInfo.subpass = m_Subpass;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkResult createGrapPipeline = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline);
		VULCANITE_CORE_ASSERT(createGrapPipeline == VK_SUCCESS, "failed to create graphics pipeline!");
		if (createGrapPipeline != VK_SUCCESS) {
			m_GraphicsPipeline = VK_NULL_HANDLE;
		}

		// module 只是创建管线的输入,建完立刻销毁
		vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
		vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
	}

	void VulkanPipeline::Destroy() {
		if (m_Device == VK_NULL_HANDLE) {
			return;
		}

		if (m_GraphicsPipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
		}
		if (m_PipelineLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
		}
		if (m_DescriptorSetLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
		}

		m_GraphicsPipeline = VK_NULL_HANDLE;
		m_PipelineLayout = VK_NULL_HANDLE;
		m_DescriptorSetLayout = VK_NULL_HANDLE;
	}

	// ===== 平台无关工厂的实现(必须放在平台层,因为要构造 VulkanPipeline) =====
	Ref<Pipeline> Pipeline::CreatePipeline(const PipelineSpe& spec) {
		Ref<VulkanPipeline> pipeline = CreateRef<VulkanPipeline>(spec);
		if (!pipeline->IsValid()) {
			VULCANITE_CORE_ERROR("创建管线失败:{0} / {1}", spec.VertexShaderPath, spec.FragmentShaderPath);
			return nullptr;
		}
		return pipeline;
	}
}
