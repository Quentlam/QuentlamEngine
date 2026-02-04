#pragma once
#include "qlpch.h"
#include "Quentlam/Renderer/Shader.h"
#include <glm/glm.hpp>

//TODO: REMOVE!!这里其实是为了下面的PreProcess（预处理）函数的编译通过而添加的。如果直接包含<glad/glad.h>后面会出现重复包含的现象。
typedef unsigned int GLenum;

namespace Quentlam
{
	class QUENTLAM_API OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name,const std::string& vertexSrc, const std::string& fragmentSrc);

		~OpenGLShader();

		void Bind()const;
		void Unbind()const;

		void UploadUniformInt(const std::string& name, int value);
		virtual const std::string& GetName() const override { return m_Name; };


		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);


		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<GLenum,std::string> PreProcess(const std::string& source);
		void Compile(std::unordered_map<GLenum, std::string>& shdaerSources);

	private:
		uint32_t m_RendererID;
		std::string m_Name;
	};
}