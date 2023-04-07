#ifndef RTESCREENSHADER_H
#define RTESCREENSHADER_H
#include "glm/glm.hpp"

namespace RTE {

	class ScreenShader {
	public:
		ScreenShader();
		virtual ~ScreenShader();

		/// <summary>
		/// Deletes the shader object. Should be called before the GL context gets destroyed.
		/// </summary>
		void Destroy();

		void Use();

		void SetInt(const std::string& uniformName, GLint value);
		void SetMatrix(const char* uniformName, const glm::mat4& value);

	private:
		/// <summary>
		/// Compile a Shader component.
		/// </summary>
		/// <param name="shaderCode">
		/// The shader code string.
		/// </param>
		/// <returns>
		/// The GL pointer to the compiled shader or 0 in case of failure.
		/// </returns>
		GLuint CompileShader(GLuint shader, const char *shaderCode, std::string& error);

		/// <summary>
		/// Link the comlete shader from vertex and fragment shader.
		/// </summary>
		/// <param name="vertexShader">
		/// The GL pointer to the vertex shader.
		/// </param>
		/// <param name="fragmentShader">
		/// The GL pointer to the fragment shader.
		/// </param>
		/// <returns>
		/// The GL pointer to the compiled and linked shader program.
		/// </returns>
		GLuint LinkShader(GLuint program, GLuint vertexShader, GLuint fragmentShader, std::string& error);

		GLuint m_ProgramID;

		/// TODO: unhardcode
		//!< Vertex shader. Simple shader to draw a texture to the entire screen.
		static constexpr const char *m_VertexShader =
			"#version 330 core\n"
			"layout(location = 0) in vec2 rtePos;\n"
			"layout(location = 1) in vec2 rteUV;\n"
			"out vec2 texCoord;\n"
			"uniform mat4 rteProjMatrix;\n"
			"uniform mat4 rteUVTransform;\n"
			"void main() {\n"
			"gl_Position = rteProjMatrix * vec4(rtePos.xy, 0.1, 1.0);\n"
			"texCoord = (rteUVTransform * vec4(rteUV, 0.0, 1.0)).st;\n"
			"}";

		//!< Fragment shader. Simple shader to draw a texture to the entire screen.
		static constexpr const char *m_FragmentShader =
			"#version 330 core\n"
			"in vec2 texCoord;\n"
			"out vec4 color;\n"
			"uniform float rteTime;\n"
			"uniform sampler2D rteTex0;\n"
			"void main() {\n"
			"color = vec4(texture(rteTex0, texCoord).rgb, 1.0);\n"
			"}";
	};
} // namespace RTE

#endif
