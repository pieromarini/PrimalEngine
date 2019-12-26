#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

#include "../../primal/core/core.h"
#include "../../primal/core/application.h"
#include "openGLShader.h"

namespace primal {

  static GLenum ShaderTypeFromString(const std::string& type) {
	if (type == "vertex")
	  return GL_VERTEX_SHADER;
	if (type == "fragment" || type == "pixel")
	  return GL_FRAGMENT_SHADER;

	PRIMAL_CORE_ASSERT(false, "Unknown shader type!");
	return 0;
  }

  OpenGLShader::OpenGLShader(const std::string& filepath) {
	PRIMAL_PROFILE_FUNCTION();

	std::string source = readFile(filepath);
	auto shaderSources = preProcess(source);
	compile(shaderSources);

	// Extract name from filepath
	auto lastSlash = filepath.find_last_of("/\\");
	lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
	auto lastDot = filepath.rfind('.');
	auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
	m_name = filepath.substr(lastSlash, count);
  }

  OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	: m_name(name) {
	PRIMAL_PROFILE_FUNCTION();

	std::unordered_map<GLenum, std::string> sources;
	sources[GL_VERTEX_SHADER] = vertexSrc;
	sources[GL_FRAGMENT_SHADER] = fragmentSrc;
	compile(sources);
  }

  OpenGLShader::~OpenGLShader() {
	PRIMAL_PROFILE_FUNCTION();

	glDeleteProgram(m_rendererID);
  }

  std::string OpenGLShader::readFile(const std::string& filepath) {
	PRIMAL_PROFILE_FUNCTION();

	std::string result;
	std::ifstream in(filepath, std::ios::in | std::ios::binary);
	if (in) {
	  in.seekg(0, std::ios::end);
	  size_t size = in.tellg();
	  if (size != -1) {
		result.resize(size);
		in.seekg(0, std::ios::beg);
		in.read(&result[0], size);
		in.close();
	  } else {
		PRIMAL_CORE_ERROR("Could not read from file '{0}'", filepath);
	  }
	} else {
	  PRIMAL_CORE_ERROR("Could not open file '{0}'", filepath);
	}

	return result;
  }

  std::unordered_map<GLenum, std::string> OpenGLShader::preProcess(const std::string& source) {
	PRIMAL_PROFILE_FUNCTION();

	std::unordered_map<GLenum, std::string> shaderSources;

	const char* typeToken = "#type";
	size_t typeTokenLength = strlen(typeToken);
	size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
	while (pos != std::string::npos) {
	  size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
	  PRIMAL_CORE_ASSERT(eol != std::string::npos, "Syntax error");
	  size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
	  std::string type = source.substr(begin, eol - begin);
	  PRIMAL_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

	  size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
	  PRIMAL_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
	  pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

	  shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
	}

	return shaderSources;
  }

  void OpenGLShader::compile(const std::unordered_map<GLenum, std::string>& shaderSources) {
	PRIMAL_PROFILE_FUNCTION();

	GLuint program = glCreateProgram();
	PRIMAL_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");

	std::array<GLenum, 2> glShaderIDs;
	int glShaderIDIndex = 0;

	for (auto& kv : shaderSources) {
	  GLenum type = kv.first;
	  const std::string& source = kv.second;

	  GLuint shader = glCreateShader(type);

	  const GLchar* sourceCStr = source.c_str();
	  glShaderSource(shader, 1, &sourceCStr, 0);

	  glCompileShader(shader);

	  GLint isCompiled = 0;
	  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	  if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

		glDeleteShader(shader);

		PRIMAL_CORE_ERROR("{0}", infoLog.data());
		PRIMAL_CORE_ASSERT(false, "Shader compilation failure!");
		break;
	  }

	  glAttachShader(program, shader);
	  glShaderIDs[glShaderIDIndex++] = shader;
	}

	m_rendererID = program;

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE) {
	  GLint maxLength = 0;
	  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

	  std::vector<GLchar> infoLog(maxLength);
	  glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

	  glDeleteProgram(program);

	  for (auto id : glShaderIDs)
		glDeleteShader(id);

	  PRIMAL_CORE_ERROR("{0}", infoLog.data());
	  PRIMAL_CORE_ASSERT(false, "Shader link failure!");
	  return;
	}

	for (auto id : glShaderIDs) {
	  glDetachShader(program, id);
	  glDeleteShader(id);
	}
  }

  void OpenGLShader::bind() const {

	glUseProgram(m_rendererID);
  }

  void OpenGLShader::unbind() const {

	glUseProgram(0);
  }

  void OpenGLShader::setInt(const std::string& name, int value) {
	PRIMAL_PROFILE_FUNCTION();

	uploadUniformInt(name, value);
  }

  void OpenGLShader::setFloat(const std::string& name, const float value) {
	PRIMAL_PROFILE_FUNCTION();

	uploadUniformFloat(name, value);
  }

  void OpenGLShader::setFloat3(const std::string& name, const glm::vec3& value) {
	PRIMAL_PROFILE_FUNCTION();

	uploadUniformFloat3(name, value);
  }

  void OpenGLShader::setFloat4(const std::string& name, const glm::vec4& value) {
	PRIMAL_PROFILE_FUNCTION();

	uploadUniformFloat4(name, value);
  }

  void OpenGLShader::setMat4(const std::string& name, const glm::mat4& value) {
	PRIMAL_PROFILE_FUNCTION();

	uploadUniformMat4(name, value);
  }

  void OpenGLShader::uploadUniformInt(const std::string& name, int value) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniform1i(location, value);
  }

  void OpenGLShader::uploadUniformFloat(const std::string& name, float value) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniform1f(location, value);
  }

  void OpenGLShader::uploadUniformFloat2(const std::string& name, const glm::vec2& value) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniform2f(location, value.x, value.y);
  }

  void OpenGLShader::uploadUniformFloat3(const std::string& name, const glm::vec3& value) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniform3f(location, value.x, value.y, value.z);
  }

  void OpenGLShader::uploadUniformFloat4(const std::string& name, const glm::vec4& value) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniform4f(location, value.x, value.y, value.z, value.w);
  }

  void OpenGLShader::uploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
  }

  void OpenGLShader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
	GLint location = glGetUniformLocation(m_rendererID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
  }

}
