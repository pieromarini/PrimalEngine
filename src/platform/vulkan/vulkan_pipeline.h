#pragma oncepipe

#include "vk_types.h"

namespace pm {

class PipelineBuilder {
	public:
		PipelineBuilder() { clear(); }
		void clear();
		void setPipelineLayout(VkPipelineLayout pipelineLayout);

		VkPipeline buildPipeline(VkDevice device);
		void setShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
		void setInputTopology(VkPrimitiveTopology topology);
		void setPolygonMode(VkPolygonMode polygonMode);
		void setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
		void setMultisamplingNone();
		void setColorAttachmentFormat(VkFormat colorFormat);
		void setDepthFormat(VkFormat depthFormat);

		void enableBlendingAdditive();
		void enableBlendingAlphablend();
		void disableBlending();

		void enableDepthTest(bool depthWriteEnable, VkCompareOp op);
		void disableDepthTest();

		VkPipelineLayout m_pipelineLayout{};

	private:
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
		VkPipelineInputAssemblyStateCreateInfo m_inputAssembly{};
		VkPipelineRasterizationStateCreateInfo m_rasterizer{};
		VkPipelineColorBlendAttachmentState m_colorBlendAttachment{};
		VkPipelineMultisampleStateCreateInfo m_multisampling{};
		VkPipelineDepthStencilStateCreateInfo m_depthStencil{};
		VkPipelineRenderingCreateInfo m_renderInfo{};
		VkFormat m_colorAttachmentformat{};
};

};// namespace pm
