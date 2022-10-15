#include "ScreenShader.h"
#include "RTEError.h"
namespace RTE{
	ScreenShader::ScreenShader() {
		std::string error;
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		vertexShader = CompileShader(vertexShader, m_VertexShader, error);
		fragmentShader = CompileShader(fragmentShader, m_FragmentShader, error);

		RTEAssert(vertexShader && fragmentShader, "Failed to compile OpenGL shaders with error: " + error);

		m_ProgramID = glCreateProgram();

		m_ProgramID = LinkShader(m_ProgramID, vertexShader, fragmentShader, error);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		RTEAssert(m_ProgramID, "Failed to link shader with error: " + error);
	}

	ScreenShader::~ScreenShader() {
		Destroy();
	}

	void ScreenShader::Destroy() {
		glDeleteProgram(m_ProgramID);
	}

	void ScreenShader::Use() {
		glUseProgram(m_ProgramID);
	}

	void ScreenShader::SetInt(const std::string& uniformName, GLint value) {
		glUseProgram(m_ProgramID);
		glUniform1i(glGetUniformLocation(m_ProgramID, uniformName.c_str()), value);
	}

	GLuint ScreenShader::CompileShader(GLuint shaderID, const char* shaderCode, std::string& error) {
		glShaderSource(shaderID, 1, &shaderCode, nullptr);
		glCompileShader(shaderID);
		GLint success;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE) {
			GLint infoLength;
			error += "\nFailed to compile shader:\n";
			size_t errorPrevLen = error.size();
			glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLength);
			error.resize(errorPrevLen + infoLength);
			glGetShaderInfoLog(shaderID, infoLength, &infoLength, error.data() + errorPrevLen);
			glDeleteShader(shaderID);
			return 0;
		}

		return shaderID;
	}

	GLuint ScreenShader::LinkShader(GLuint program, GLuint vertexShader, GLuint fragmentShader, std::string& error) {
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glLinkProgram(program);

		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);

		GLint success;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (success == GL_FALSE) {
			GLint infoLength;
			error += "\nFailed to link program:\n";
			size_t errorPrevLen = error.size();
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
			error.resize(errorPrevLen + infoLength);
			glGetProgramInfoLog(program, infoLength, &infoLength, error.data() + errorPrevLen);
			glDeleteProgram(program);
			return 0;
		}

		return program;
	}

}
