#include "ScreenShader.h"
#include "RTEError.h"
#include "glm/gtc/type_ptr.hpp"
namespace RTE{
	ScreenShader::ScreenShader() {
		std::string error;
		GLuint vertexShader = GL_CHECK(glCreateShader(GL_VERTEX_SHADER););
		GLuint fragmentShader = GL_CHECK(glCreateShader(GL_FRAGMENT_SHADER););
		vertexShader = CompileShader(vertexShader, m_VertexShader, error);
		fragmentShader = CompileShader(fragmentShader, m_FragmentShader, error);

		RTEAssert(vertexShader && fragmentShader, "Failed to compile OpenGL shaders with error: " + error);

		m_ProgramID = GL_CHECK(glCreateProgram(););

		m_ProgramID = LinkShader(m_ProgramID, vertexShader, fragmentShader, error);
		GL_CHECK(glDeleteShader(vertexShader););
		GL_CHECK(glDeleteShader(fragmentShader););

		RTEAssert(m_ProgramID, "Failed to link shader with error: " + error);
	}

	ScreenShader::~ScreenShader() {
		Destroy();
	}

	void ScreenShader::Destroy() {
		GL_CHECK(glDeleteProgram(m_ProgramID););
	}

	void ScreenShader::Use() {
		GL_CHECK(glUseProgram(m_ProgramID););
	}

	void ScreenShader::SetInt(const std::string& uniformName, GLint value) {
		GL_CHECK(glUseProgram(m_ProgramID););
		GL_CHECK(glUniform1i(glGetUniformLocation(m_ProgramID, uniformName.c_str()), value););
	}

	void ScreenShader::SetMatrix(const char* uniformName, const glm::mat4 &value) {
		GL_CHECK(glUseProgram(m_ProgramID););
		GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(m_ProgramID, uniformName), 1, GL_FALSE, glm::value_ptr(value)););
	}

	GLuint ScreenShader::CompileShader(GLuint shaderID, const char* shaderCode, std::string& error) {
		GL_CHECK(glShaderSource(shaderID, 1, &shaderCode, nullptr););
		GL_CHECK(glCompileShader(shaderID););
		GLint success;
		GL_CHECK(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success););

		if (success == GL_FALSE) {
			GLint infoLength;
			error += "\nFailed to compile shader:\n";
			size_t errorPrevLen = error.size();
			GL_CHECK(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLength););
			error.resize(errorPrevLen + infoLength);
			GL_CHECK(glGetShaderInfoLog(shaderID, infoLength, &infoLength, error.data() + errorPrevLen););
			GL_CHECK(glDeleteShader(shaderID););
			return 0;
		}

		return shaderID;
	}

	GLuint ScreenShader::LinkShader(GLuint program, GLuint vertexShader, GLuint fragmentShader, std::string& error) {
		GL_CHECK(glAttachShader(program, vertexShader););
		GL_CHECK(glAttachShader(program, fragmentShader););

		GL_CHECK(glLinkProgram(program););

		GL_CHECK(glDetachShader(program, vertexShader););
		GL_CHECK(glDetachShader(program, fragmentShader););

		GLint success;
		GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &success););

		if (success == GL_FALSE) {
			GLint infoLength;
			error += "\nFailed to link program:\n";
			size_t errorPrevLen = error.size();
			GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength););
			error.resize(errorPrevLen + infoLength);
			GL_CHECK(glGetProgramInfoLog(program, infoLength, &infoLength, error.data() + errorPrevLen););
			GL_CHECK(glDeleteProgram(program););
			return 0;
		}

		return program;
	}

}
