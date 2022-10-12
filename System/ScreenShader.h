#ifndef RTESCREENSHADER_H
#define RTESCREENSHADER_H
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
		static constexpr const char *m_VertexShader = ""
			"#version 330 core\n"
			"layout(location = 0) in vec2 aPos;"
			"layout(location = 1) in vec2 texST;"
			"out vec2 texCoord;"
			"void main() {"
			"gl_Position = vec4(aPos.xy, 0.0, 1.0);"
			"texCoord = texST;"
			"};";

		//!< Fragment shader. Simple shader to draw a texture to the entire screen.
		static constexpr const char *m_FragmentShader = ""
			"#version 330 core\n"
			"in vec2 texCoord;"
			"out vec4 color;"
			"uniform float time;"
			"uniform sampler2D tex;"
			"void main() {"
			"color = vec4(texture(tex, texCoord).rgb, 1.0);"
			"}";
	};
} // namespace RTE

#endif
