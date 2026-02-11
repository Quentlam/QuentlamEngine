#pragma once
#include "qlpch.h"
#include <glm/glm.hpp>



namespace Quentlam
{
	class QUENTLAM_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual const std::string& GetName() const = 0;


		virtual void SetInt(const std::string& name, const int value) = 0;
		virtual void SetFloat(const std::string& name, const float value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;


		static Ref<Shader> Create(const std::string& filepath);
		static Ref<Shader> Create(const std::string& name,const std::string& vertexSrc, const std::string& fragmentSrc);
		

	};


	class QUENTLAM_API ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);//添加已有的shader
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);//添加shader，这个构造方法初始化时，会把shader的名字做成文件名字
		Ref<Shader> Load(const std::string& name,const std::string& filepath);//添加shader，这个构造可以自定义shader的名字
		Ref<Shader> Get(const std::string& name);

		bool Exist(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;//如果在sandbox中不创建一个ShaderLibrary实例，那么这个表直接空的

	};
}