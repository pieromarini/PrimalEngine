#include "renderer2D.h"

#include "vertexArray.h"
#include "shader.h"
#include "renderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace primal {

	struct Renderer2DStorage {
		ref_ptr<VertexArray> quadVertexArray;
		ref_ptr<Shader> textureShader;
		ref_ptr<Texture2D> whiteTexture;
	};

	static Renderer2DStorage* s_data;

	void Renderer2D::init() {

		s_data = new Renderer2DStorage();
		s_data->quadVertexArray = VertexArray::create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		ref_ptr<VertexBuffer> squareVB = VertexBuffer::create(squareVertices, sizeof(squareVertices));
		squareVB->setLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		s_data->quadVertexArray->addVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		ref_ptr<IndexBuffer> squareIB = IndexBuffer::create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		s_data->quadVertexArray->setIndexBuffer(squareIB);

		s_data->whiteTexture = Texture2D::create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_data->whiteTexture->setData(&whiteTextureData, sizeof(uint32_t));

		s_data->textureShader = Shader::create("res/shaders/texture.glsl");
		s_data->textureShader->bind();
		s_data->textureShader->setInt("u_Texture", 0);
	}

	void Renderer2D::shutdown() {
		delete s_data;
	}

	void Renderer2D::beginScene(const OrthographicCamera& camera) {
		s_data->textureShader->bind();
		s_data->textureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
	}

	void Renderer2D::endScene() { }

	void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
		drawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {

		s_data->textureShader->setFloat4("u_Color", color);
		s_data->whiteTexture->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_data->textureShader->setMat4("u_Transform", transform);
		s_data->quadVertexArray->bind();
		RenderCommand::drawIndexed(s_data->quadVertexArray);
	}

	void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const ref_ptr<Texture2D>& texture) {
		drawQuad({ position.x, position.y, 0.0f }, size, texture);
	}

	void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const ref_ptr<Texture2D>& texture) {
		s_data->textureShader->setFloat4("u_Color", glm::vec4(1.0f));
		texture->bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_data->textureShader->setMat4("u_Transform", transform);

		s_data->quadVertexArray->bind();
		RenderCommand::drawIndexed(s_data->quadVertexArray);
	}

}
