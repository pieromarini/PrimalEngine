#include <glad/glad.h>

#include "opengl_shader.h"
#include "tools/log.h"

namespace primal::renderer::opengl {

  OpenGLShader::OpenGLShader(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines) {
	load(name, vsCode, fsCode, defines);
  }

  void OpenGLShader::load(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines) {
	m_name = name;

	auto vs = glCreateShader(GL_VERTEX_SHADER);
	auto fs = glCreateShader(GL_FRAGMENT_SHADER);

	m_id = glCreateProgram();
	int status;
	char log[1024];

	if (defines.size() > 0) {
	  std::vector<std::string> vsMergedCode;
	  std::vector<std::string> fsMergedCode;

	  std::string firstLine = vsCode.substr(0, vsCode.find("\n"));

	  if (firstLine.find("#version") != std::string::npos) {
		vsCode = vsCode.substr(vsCode.find("\n") + 1, vsCode.length() - 1);
		vsMergedCode.push_back(firstLine + "\n");
	  }

	  firstLine = fsCode.substr(0, fsCode.find("\n"));
	  if (firstLine.find("#version") != std::string::npos) {
		fsCode = fsCode.substr(fsCode.find("\n") + 1, fsCode.length() - 1);
		fsMergedCode.push_back(firstLine + "\n");
	  }

	  for (unsigned int i = 0; i < defines.size(); ++i) {
		std::string define = "#define " + defines[i] + "\n";
		vsMergedCode.push_back(define);
		fsMergedCode.push_back(define);
	  }

	  vsMergedCode.push_back(vsCode);
	  fsMergedCode.push_back(fsCode);

	  const char** vsStringsC = new const char*[vsMergedCode.size()];
	  const char** fsStringsC = new const char*[fsMergedCode.size()];

	  for (unsigned int i = 0; i < vsMergedCode.size(); ++i)
		vsStringsC[i] = vsMergedCode[i].c_str();
	  for (unsigned int i = 0; i < fsMergedCode.size(); ++i)
		fsStringsC[i] = fsMergedCode[i].c_str();

	  glShaderSource(vs, vsMergedCode.size(), vsStringsC, NULL);
	  glShaderSource(fs, fsMergedCode.size(), fsStringsC, NULL);

	  delete[] vsStringsC;
	  delete[] fsStringsC;
	} else {
	  const char* vsSourceC = vsCode.c_str();
	  const char* fsSourceC = fsCode.c_str();

	  glShaderSource(vs, 1, &vsSourceC, NULL);
	  glShaderSource(fs, 1, &fsSourceC, NULL);
	}

	glCompileShader(vs);
	glCompileShader(fs);

	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status) {
	  glGetShaderInfoLog(vs, 1024, NULL, log);
	  PRIMAL_CORE_ERROR("Vertex shader compilation error at: {0}", name);
	}

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (!status) {
	  glGetShaderInfoLog(fs, 1024, NULL, log);
	  PRIMAL_CORE_ERROR("Fragment shader compilation error at: {0}", name);
	}

	glAttachShader(m_id, vs);
	glAttachShader(m_id, fs);
	glLinkProgram(m_id);

	glGetProgramiv(m_id, GL_LINK_STATUS, &status);
	if (!status) {
	  glGetProgramInfoLog(m_id, 1024, NULL, log);
	  PRIMAL_CORE_ERROR("Shader program linking error:\n {0}", log);
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	int nrAttributes, nrUniforms;
	glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &nrAttributes);
	glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &nrUniforms);
	m_attributes.resize(nrAttributes);
	m_uniforms.resize(nrUniforms);

	char buffer[128];
	for (unsigned int i = 0; i < nrAttributes; ++i) {
	  GLenum glType;
	  glGetActiveAttrib(m_id, i, sizeof(buffer), nullptr, &m_attributes[i].size, &glType, buffer);
	  m_attributes[i].name = std::string(buffer);
	  m_attributes[i].type = SHADER_TYPE_BOOL;

	  m_attributes[i].location = glGetAttribLocation(m_id, buffer);
	}

	for (unsigned int i = 0; i < nrUniforms; ++i) {
	  GLenum glType;
	  glGetActiveUniform(m_id, i, sizeof(buffer), 0, &m_uniforms[i].size, &glType, buffer);
	  m_uniforms[i].name = std::string(buffer);
	  m_uniforms[i].type = SHADER_TYPE_BOOL;

	  m_uniforms[i].location = glGetUniformLocation(m_id, buffer);
	}
  }

  void OpenGLShader::use() {
	glUseProgram(m_id);
  }

  bool OpenGLShader::hasUniform(std::string name) {
	for (const auto& m_uniform : m_uniforms) {
	  if (m_uniform.name == name) {
		return true;
	  }
	}
	return false;
  }

  void OpenGLShader::setInt(std::string location, int value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform1i(loc, value);
  }

  void OpenGLShader::setBool(std::string location, bool value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform1i(loc, (int)value);
  }

  void OpenGLShader::setFloat(std::string location, float value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform1f(loc, (int)value);
  }

  // void setVector(std::string location, math::Vector2 value);

  void OpenGLShader::setVector(std::string location, math::Vector3 value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform3fv(loc, 1, &value[0]);
  }

  void OpenGLShader::setVector(std::string location, math::Vector4 value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform4fv(loc, 1, &value[0]);
  }

  // void setVectorArray(std::string location, int size, const std::vector<math::vec2>& values);

  void OpenGLShader::setVectorArray(std::string location, std::size_t size, std::vector<math::Vector3>& values) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform3fv(loc, size, (float*)(&values[0].x));
  }

  void OpenGLShader::setVectorArray(std::string location, std::size_t size, std::vector<math::Vector4>& values) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniform4fv(loc, size, (float*)(&values[0].x));
  }

  // void setMatrix(std::string location, math::mat2 value);

  void OpenGLShader::setMatrix(std::string location, math::Matrix3 value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]);
  }

  void OpenGLShader::setMatrix(std::string location, math::Matrix4 value) {
	auto loc = getUniformLocation(location);
	if (loc >= 0)
	  glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
  }

  int OpenGLShader::getUniformLocation(std::string name) {
	for (const auto& uniform : m_uniforms) {
	  if (uniform.name == name) {
		return uniform.location;
	  }
	}
	return -1;
  }

}// namespace primal::renderer::opengl
