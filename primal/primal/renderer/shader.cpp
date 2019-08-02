#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "../log.h"

#include "shader.h"

namespace primal {

  Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
	// create an empty vertex shader handle
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Send the vertex shader source code to GL
	// Note that std::string's .c_str is NULL character terminated.
	const GLchar* source = vertexSrc.c_str();
	glShaderSource(vertexShader, 1, &source, 0);

	// Compile the vertex shader
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

	  // The maxLength includes the NULL character
	  std::vector<GLchar> infoLog(maxLength);
	  glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

	  // We don't need the shader anymore.
	  glDeleteShader(vertexShader);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Vertex shader compilation failure!");
	  return;
	}

	// create an empty fragment shader handle
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Send the fragment shader source code to GL
	// Note that std::string's .c_str is NULL character terminated.
	source = fragmentSrc.c_str();
	glShaderSource(fragmentShader, 1, &source, 0);

	// Compile the fragment shader
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

	  // The maxLength includes the NULL character
	  std::vector<GLchar> infoLog(maxLength);
	  glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

	  // We don't need the shader anymore.
	  glDeleteShader(fragmentShader);
	  // Either of them. Don't leak shaders.
	  glDeleteShader(vertexShader);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Fragment shader compilation failure!");
	  return;
	}

	// Vertex and fragment shaders are successfully compiled.
	// Now time to link them together into a program.
	// Get a program object.
	m_RendererID = glCreateProgram();
	GLuint program = m_RendererID;

	// Attach our shaders to our program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// Link our program
	glLinkProgram(program);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

	  // The maxLength includes the NULL character
	  std::vector<GLchar> infoLog(maxLength);
	  glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

	  // We don't need the program anymore.
	  glDeleteProgram(program);
	  // Don't leak shaders either.
	  glDeleteShader(vertexShader);
	  glDeleteShader(fragmentShader);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Shader link failure!");
	  return;
	}

	// Always detach shaders after a successful link.
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);
  }

  Shader::~Shader() {
	glDeleteProgram(m_RendererID);
  }

  void Shader::bind() const {
	glUseProgram(m_RendererID);
  }

  void Shader::unbind() const {
	glUseProgram(0);
  }

  void Shader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
  }

}
