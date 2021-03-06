#pragma once
#ifdef ENABLE_OPENGL

namespace Angazi::Graphics
{
	class Shader
	{
	public:
		void Initialize(const std::filesystem::path filePath, const char * VSshaderName = "VS", const char * PSshaderName = "PS");
		void Terminate();
		void Bind();

		~Shader();

		//set uniforms
		void SetUniform1i(const std::string name, int value);
		void SetUniform1f(const std::string name, float value);
		void SetUniform3f(const std::string name, Math::Vector3 vec);
		void SetUniform4f(const std::string name, Math::Vector4 vec);
		void SetUniform4f(const std::string name, float v0, float v1, float v2, float v3);
		void SetUniformMat4f(const std::string name, Math::Matrix4 mat);

	private:
		unsigned int CompileShader(unsigned int type, const std::string& source);
		std::vector<std::string> ParseShader(const std::filesystem::path& filepath, const char * VSshaderName, const char * PSshaderName);
		unsigned int mProgram;
	};
}
#endif