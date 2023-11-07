#ifndef _RTE_SHADER_
#define _RTE_SHADER_

#include "Serializable.h"
#include "glm/glm.hpp"

namespace RTE {
	class Shader {
	public:

		/// <summary> 
		/// Constructs an empty shader program, which can be initialized using `Shader::Compile`
		/// </summary>
		Shader();
		
		/// <summary> 
		/// Constructs a Shader from vertex shader file and fragment shader file.
		/// </summary>
		/// <param name="vertexFilename">
		/// Filepath to the vertex shader file.
		/// </param>
		/// <param name="fragPath">
		/// Filepath to the fragment shader file. 
		/// </param>
		Shader(const std::string &vertexFilename, const std::string &fragPath);
		
		/// <summary>
		/// Destructor.
		/// </summary>
		virtual ~Shader();

		/// <summary>
		/// Create this shader from a vertex shader file and a fragment shader file.
		/// </summary>
		/// <param name="vertexFilename">
		/// Filepath to the vertex shader.
		/// </param>
		/// <param name="fragPath">
		/// Filepath to the fragment shader
		/// </param>
		bool Compile(const std::string &vertexFilename, const std::string &fragPath);

		void Use();
#pragma region Uniform handling
		/// <summary>
		/// Returns the location of a uniform given by name.
		/// </summary>
		/// <param name="name">
		/// String containing the name of a uniform in this shader program.
		/// </param>
		/// <returns>
		/// A GLint containing the location of the requested uniform.
		/// </returns>
		int32_t GetUniformLocation(const std::string &name);

		/// <summary>
		/// Set a boolean uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The boolean value to set the uniform to.
		/// </param>
		void SetBool(const std::string &name, bool value);

		/// <summary>
		/// Set an integer uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The integer value to set the uniform to.
		/// </param>
		void SetInt(const std::string &name, int value);

		/// <summary>
		/// Set a float uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float value to set the uniform to.
		/// </param>
		void SetFloat(const std::string &name, float value);

		/// <summary>
		/// Set a float mat4 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float mat4 value to set the uniform to.
		/// </param>
		void SetMatrix4f(const std::string &name, const glm::mat4 &value);

		/// <summary>
		/// Set a float vec2 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec2 value to set the uniform to.
		/// </param>
		void SetVector2f(const std::string &name, const glm::vec2 &value);

		/// <summary>
		/// Set a float vec3 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec3 value to set the uniform to.
		/// </param>
		void SetVector3f(const std::string &name, const glm::vec3 &value);

		/// <summary>
		/// Set a float vec4 uniform value in the active program by name.
		/// </summary>
		/// <param name="name">
		/// The name of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec4 value to set the uniform to.
		/// </param>
		void SetVector4f(const std::string &name, const glm::vec4 &value);

		/// <summary>
		/// Set a boolean uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The boolean value to set the uniform to.
		/// </param>
		static void SetBool(int32_t uniformLoc, bool value);

		/// <summary>
		/// Set an integer uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The integer value to set the uniform to.
		/// </param>
		static void SetInt(int32_t uniformLoc, int value);

		/// <summary>
		/// Set a float uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float value to set the uniform to.
		/// </param>
		static void SetFloat(int32_t uniformLoc, float value);

		/// <summary>
		/// Set a float mat4 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float mat4 value to set the uniform to.
		/// </param>
		static void SetMatrix4f(int32_t uniformLoc, const glm::mat4 &value);

		/// <summary>
		/// Set a float vec2 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec2 value to set the uniform to.
		/// </param>
		static void SetVector2f(int32_t uniformLoc, const glm::vec2 &value);

		/// <summary>
		/// Set a float vec3 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec3 value to set the uniform to.
		/// </param>
		static void SetVector3f(int32_t uniformLoc, const glm::vec3 &value);

		/// <summary>
		/// Set a float vec4 uniform value in the active program by location.
		/// </summary>
		/// <param name="uniformLoc">
		/// The location of the uniform to set.
		/// </param>
		/// <param name="value">
		/// The float vec4 value to set the uniform to.
		/// </param>
		static void SetVector4f(int32_t uniformLoc, const glm::vec4 &value);
#pragma endregion

#pragma region Engine Defined Uniforms

		/// <summary>
		/// Get the location of the texture sampler uniform.
		/// </summary>
		/// <returns>
		/// The location of the texture uniform. This may be -1 if the shader doesn't use textures, in which case the value will be ignored.
		/// </returns>
		int GetTextureUniform() { return m_TextureUniform; }

		/// <summary>
		/// Get the location of the color modifier uniform.
		/// </summary>
		/// <returns>
		/// The location of the color modifier uniform. This may be -1 if the shader doesn't use the color mod, in which case the value will  be ignored.
		/// </returns>
		int GetColorUniform() { return m_ColorUniform; }

		/// <summary>
		/// Get the location of the transformation matrix uniform.
		/// </summary>
		/// <returns>
		/// The location of the transformation matrix uniform. This may be -1 if the shader doesn't use transforms, in which case the value will be ignored.
		/// </returns>
		int GetTransformUniform() { return m_TransformUniform; }

		/// <summary>
		/// Get the location of the transformation matrix uniform.
		/// </summary>
		/// <returns>
		/// The location of the UV transformation matrix uniform. This may be -1 if the shader doesn't use UV transforms, in which case the value will be ignored.
		/// </returns>
		int GetUVTransformUniform() { return m_UVTransformUniform; }

		/// <summary>
		/// Get the location of the projection matrix uniform.
		/// </summary>
		/// <returns>
		/// The location of the color modifier uniform. This may be -1 if the shader doesn't apply projection, in which case the value will be ignored.
		/// </returns>
		int GetProjectionUniform() { return m_ProjectionUniform; }

#pragma endregion
	private:
		uint32_t m_ProgramID;

		/// <summary>
		/// Compiles a shader component from a data string.
		/// </summary>
		/// <param name="shaderID">
		/// ID of the shader component to compile.
		/// </param>
		/// <param name="data">
		/// The shader code string.
		/// </param>
		/// <param name="error">
		/// String to contain error data returned by opengl during compilation.
		/// </param>
		/// <returns>
		/// Whether compilation was successful.
		/// </returns>
		bool CompileShader(uint32_t shaderID, const std::string &data, std::string &error);

		/// <summary> 
		/// Links a shader program from a vertex and fragment shader.
		/// </summary>
		/// <param name="vtxShader">
		/// The compiled vertex shader.
		/// </param>
		/// <param name="fragShader">
		/// The compiled fragment shader.
		/// </param>
		/// <returns>
		/// Whether linking was successful.
		/// </returns>
		bool Link(uint32_t vtxShader, uint32_t fragShader);

		/// <summary>
		/// Sets default values for the shader uniforms (may not persist across frames!)
		/// </summary>
		void ApplyDefaultUniforms();

		int m_TextureUniform; //!< Location of the texture uniform (sampler2d rteTexture).
		int m_ColorUniform; //!< Location of the colormod uniform (vec4 rteColor).
		int m_TransformUniform; //!< Location of the transform uniform (mat4 rteTransform).
		int m_UVTransformUniform; //!< Location of the UV transform uniform (mat4 rteUVTransform).
		int m_ProjectionUniform; //!< Location of the projection uniform (mat4 rteProjection).
	};
} // namespace RTE
#endif
