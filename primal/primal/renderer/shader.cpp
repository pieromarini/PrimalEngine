#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "../core/log.h"

#include "shader.h"

namespace primal {

  Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	const GLchar* source = vertexSrc.c_str();
	glShaderSource(vertexShader, 1, &source, 0);

	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

	  std::vector<GLchar> infoLog(maxLength);
	  glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

	  // We don't need the shader anymore.
	  glDeleteShader(vertexShader);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Vertex shader compilation failure!");
	  return;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	source = fragmentSrc.c_str();
	glShaderSource(fragmentShader, 1, &source, 0);

	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

	  std::vector<GLchar> infoLog(maxLength);
	  glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

	  glDeleteShader(fragmentShader);
	  glDeleteShader(vertexShader);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Fragment shader compilation failure!");
	  return;
	}

	m_rendererID = glCreateProgram();
	GLuint program = m_rendererID;

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, static_cast<int*>(&isLinked));
	if (isLinked == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

	  std::vector<GLchar> infoLog(maxLength);
	  glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

	  glDeleteProgram(program);
	  glDeleteShader(vertexShader);
	  glDeleteShader(fragmentShader);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Shader link failure!");
	  return;
	}

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
  }

  Shader::~Shader() {
	glDeleteProgram(m_rendererID);
  }

  void Shader::bind() const {
	glUseProgram(m_rendererID);
  }

  void Shader::unbind() const {
	glUseProgram(0);
  }

  void Shader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
  }

}
