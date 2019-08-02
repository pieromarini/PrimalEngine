#include "../primal/primal.h"
#include "../vendor/imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer : public primal::Layer {
  public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f) {

	  m_VertexArray.reset(primal::VertexArray::create());

	  float vertices[3 * 7] = {
		-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
		0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
		0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
	  };

	  std::shared_ptr<primal::VertexBuffer> vertexBuffer;
	  vertexBuffer.reset(primal::VertexBuffer::create(vertices, sizeof(vertices)));
	  primal::BufferLayout layout = {
		{ primal::ShaderDataType::Float3, "a_Position" },
		{ primal::ShaderDataType::Float4, "a_Color" }
	  };
	  vertexBuffer->setLayout(layout);
	  m_VertexArray->addVertexBuffer(vertexBuffer);

	  uint32_t indices[3] = { 0, 1, 2 };
	  std::shared_ptr<primal::IndexBuffer> indexBuffer;
	  indexBuffer.reset(primal::IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t)));
	  m_VertexArray->setIndexBuffer(indexBuffer);

	  m_SquareVA.reset(primal::VertexArray::create());

	  float squareVertices[3 * 4] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	  };

	  std::shared_ptr<primal::VertexBuffer> squareVB;
	  squareVB.reset(primal::VertexBuffer::create(squareVertices, sizeof(squareVertices)));

	  squareVB->setLayout({
		{ primal::ShaderDataType::Float3, "a_Position" }
	  });

	  m_SquareVA->addVertexBuffer(squareVB);

	  uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
	  std::shared_ptr<primal::IndexBuffer> squareIB;
	  squareIB.reset(primal::IndexBuffer::create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
	  m_SquareVA->setIndexBuffer(squareIB);

	  std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec3 v_Position;
			out vec4 v_Color;
			void main() {
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

	  std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			in vec4 v_Color;
			void main() {
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

	  m_Shader.reset(new primal::Shader(vertexSrc, fragmentSrc));

	  std::string blueShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			out vec3 v_Position;
			void main() {
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);	
			}
		)";

	  std::string blueShaderFragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			void main() {
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

	  m_BlueShader.reset(new primal::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	void onUpdate(primal::Timestep ts) override {
	  if (primal::Input::isKeyPressed(PRIMAL_KEY_LEFT))
		m_CameraPosition.x -= m_CameraMoveSpeed * ts;
	  else if (primal::Input::isKeyPressed(PRIMAL_KEY_RIGHT))
		m_CameraPosition.x += m_CameraMoveSpeed * ts;

	  if (primal::Input::isKeyPressed(PRIMAL_KEY_UP))
		m_CameraPosition.y += m_CameraMoveSpeed * ts;
	  else if (primal::Input::isKeyPressed(PRIMAL_KEY_DOWN))
		m_CameraPosition.y -= m_CameraMoveSpeed * ts;

	  if (primal::Input::isKeyPressed(PRIMAL_KEY_A))
		m_CameraRotation += m_CameraRotationSpeed * ts;
	  if (primal::Input::isKeyPressed(PRIMAL_KEY_D))
		m_CameraRotation -= m_CameraRotationSpeed * ts;

	  primal::RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	  primal::RenderCommand::clear();

	  m_Camera.setPosition(m_CameraPosition);
	  m_Camera.setRotation(m_CameraRotation);

	  primal::Renderer::beginScene(m_Camera);

	  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

	  for (int y = 0; y < 20; y++) {
		for (int x = 0; x < 20; x++) {
		  glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
		  glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
		  primal::Renderer::submit(m_BlueShader, m_SquareVA, transform);
		}
	  }

	  primal::Renderer::submit(m_Shader, m_VertexArray);

	  primal::Renderer::endScene();
	}

	virtual void onImGuiRender() override {

	}

	void onEvent(primal::Event& event) override { 

	}
  private:
	std::shared_ptr<primal::Shader> m_Shader;
	std::shared_ptr<primal::VertexArray> m_VertexArray;

	std::shared_ptr<primal::Shader> m_BlueShader;
	std::shared_ptr<primal::VertexArray> m_SquareVA;

	primal::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;
};

class Sandbox : public primal::Application {
  public:
	Sandbox() {
	  pushLayer(new ExampleLayer());
	}

	~Sandbox() { }
};

primal::Application* primal::createApplication() {
  return new Sandbox();
}
