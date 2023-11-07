#include "Shader.h"
#include "glad/gl.h"
#include "glm/gtc/type_ptr.hpp"
#include "GLCheck.h"

#include "Managers/ConsoleMan.h"
#include "System/System.h"
#include "RTEError.h"

namespace RTE {

	Shader::Shader() :
	    m_ProgramID(0), m_TextureUniform(-1), m_ColorUniform(-1), m_TransformUniform(-1), m_ProjectionUniform(-1) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Shader::Shader(const std::string &vertexFilename, const std::string &fragPath) :
	    m_ProgramID(glCreateProgram()), m_TextureUniform(-1), m_ColorUniform(-1), m_TransformUniform(-1), m_ProjectionUniform(-1) {
		Compile(vertexFilename, fragPath);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Shader::~Shader() {
		if (m_ProgramID) {
			GL_CHECK(glDeleteProgram(m_ProgramID));
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Shader::Compile(const std::string &vertexPath, const std::string &fragPath) {
		assert(m_ProgramID != 0);
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		bool result{false};

		std::string error;
		result = CompileShader(vertexShader, vertexPath, error) && CompileShader(fragmentShader, fragPath, error);
		if (result) {
			GL_CHECK(glBindAttribLocation(m_ProgramID, 0, "rteVertexPosition"));
			GL_CHECK(glBindAttribLocation(m_ProgramID, 1, "rteVertexTexUV"));
			GL_CHECK(glBindAttribLocation(m_ProgramID, 2, "rteVertexColor"));
			if (Link(vertexShader, fragmentShader)) {
				m_TextureUniform = GetUniformLocation("rteTexture");
				m_ColorUniform = GetUniformLocation("rteColor");
				m_TransformUniform = GetUniformLocation("rteTransform");
				m_UVTransformUniform = GetUniformLocation("rteUVTransform");
				m_ProjectionUniform = GetUniformLocation("rteProjection");
			} else {
			}
		} else {
			GL_CHECK(glDeleteShader(vertexShader));
			GL_CHECK(glDeleteShader(fragmentShader));

			RTEAbort("ERROR: Failed to compile shaders:\n" + error);
			return false;
		}

		ApplyDefaultUniforms();

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::Use() { GL_CHECK(glUseProgram(m_ProgramID)); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GLint Shader::GetUniformLocation(const std::string &name) { return glGetUniformLocation(m_ProgramID, name.c_str()); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetBool(const std::string &name, bool value) { GL_CHECK(glUniform1i(glGetUniformLocation(m_ProgramID, name.c_str()), static_cast<int>(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetInt(const std::string &name, int value) { GL_CHECK(glUniform1i(glGetUniformLocation(m_ProgramID, name.c_str()), value)); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetFloat(const std::string &name, float value) { GL_CHECK(glUniform1f(glGetUniformLocation(m_ProgramID, name.c_str()), value)); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetMatrix4f(const std::string &name, const glm::mat4 &value) { GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(m_ProgramID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetVector2f(const std::string &name, const glm::vec2 &value) { GL_CHECK(glUniform2fv(glGetUniformLocation(m_ProgramID, name.c_str()), 1, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetVector3f(const std::string &name, const glm::vec3 &value) { GL_CHECK(glUniform3fv(glGetUniformLocation(m_ProgramID, name.c_str()), 1, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetVector4f(const std::string &name, const glm::vec4 &value) { GL_CHECK(glUniform4fv(glGetUniformLocation(m_ProgramID, name.c_str()), 1, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetBool(int32_t uniformLoc, bool value) { GL_CHECK(glUniform1i(uniformLoc, value)); }

	void Shader::SetInt(int32_t uniformLoc, int value) { GL_CHECK(glUniform1i(uniformLoc, value)); }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void Shader::SetFloat(int32_t uniformLoc, float value) { GL_CHECK(glUniform1f(uniformLoc, value)); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetMatrix4f(int32_t uniformLoc, const glm::mat4 &value) { GL_CHECK(glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetVector2f(int32_t uniformLoc, const glm::vec2 &value) { GL_CHECK(glUniform2fv(uniformLoc, 1, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetVector3f(int32_t uniformLoc, const glm::vec3 &value) { GL_CHECK(glUniform3fv(uniformLoc, 1, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::SetVector4f(int32_t uniformLoc, const glm::vec4 &value) { GL_CHECK(glUniform4fv(uniformLoc, 1, glm::value_ptr(value))); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Shader::CompileShader(GLuint shaderID, const std::string &filename, std::string &error) {
		if (!System::PathExistsCaseSensitive(filename)) {
			error += "File " + filename + " doesn't exist.";
			return false;
		}

		std::ifstream file(filename);
		std::ostringstream dataStream;

		dataStream << file.rdbuf();

		std::string dataString = dataStream.str();

		const char *data = dataString.c_str();

		GL_CHECK(glShaderSource(shaderID, 1, &data, nullptr));
		GL_CHECK(glCompileShader(shaderID));
		GLint success;
		GL_CHECK(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success));

		if (success == GL_FALSE) {
			GLint infoLength;
			error += "\nFailed to compile " + filename + ":\n";
			size_t errorPrevLen = error.size();
			GL_CHECK(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLength));
			error.resize(errorPrevLen + infoLength);
			GL_CHECK(glGetShaderInfoLog(shaderID, infoLength, &infoLength, error.data() + errorPrevLen));
			return false;
		}

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool Shader::Link(GLuint vtxShader, GLuint fragShader) {
		GL_CHECK(glAttachShader(m_ProgramID, vtxShader));
		GL_CHECK(glAttachShader(m_ProgramID, fragShader));

		GL_CHECK(glLinkProgram(m_ProgramID));

		GL_CHECK(glDetachShader(m_ProgramID, vtxShader));
		GL_CHECK(glDetachShader(m_ProgramID, fragShader));
		GL_CHECK(glDeleteShader(vtxShader));
		GL_CHECK(glDeleteShader(fragShader));

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void Shader::ApplyDefaultUniforms(){
		Use();
		SetInt("rteTexture", 0);
		SetInt("rtePalette", 1);
		SetVector4f("rteColor", glm::vec4(1));
		SetMatrix4f("rteTransform", glm::mat4(1));
		SetMatrix4f("rteProjection", glm::mat4(1));
	}
} // namespace RTE