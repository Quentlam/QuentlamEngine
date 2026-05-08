#pragma once

#include "Quentlam/Core/Base.h"
#include "Quentlam/Renderer/Shader.h"
#include "Quentlam/Renderer/Texture.h"

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Quentlam {

	// Abstract Render Primitive Interface (For decoupled Entity -> Shader binding)
	class QUENTLAM_API IRenderPrimitive
	{
	public:
		virtual ~IRenderPrimitive() = default;
		virtual void Draw() const = 0;
	};

	// Abstract Shader Interface (Extended from existing Shader)
	class QUENTLAM_API IShader
	{
	public:
		virtual ~IShader() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		
		// Variant Management & Compilation Cache hooks
		virtual void Recompile(const std::vector<std::string>& macros) = 0;
		virtual bool IsValid() const = 0;
	};

	// Base Material Interface
	class QUENTLAM_API IMaterial
	{
	public:
		virtual ~IMaterial() = default;
		virtual Ref<Shader> GetShader() const = 0;
		virtual void Bind() const = 0;
	};

	// Material Template (Engine Layer)
	// Holds the base shader and default parameters. Highly data-driven.
	class QUENTLAM_API Material : public IMaterial
	{
	public:
		Material(const Ref<Shader>& shader, const std::string& name = "Material");
		virtual ~Material() = default;

		virtual Ref<Shader> GetShader() const override { return m_Shader; }
		virtual void Bind() const override;

		const std::string& GetName() const { return m_Name; }

		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "Material");

	protected:
		Ref<Shader> m_Shader;
		std::string m_Name;
	};

	// Material Instance (Engine Layer)
	// Holds overrides for specific parameters. Thread-safe for background loading.
	class QUENTLAM_API MaterialInstance : public IMaterial
	{
	public:
		MaterialInstance(const Ref<Material>& baseMaterial, const std::string& name = "MaterialInstance");
		virtual ~MaterialInstance() = default;

		virtual Ref<Shader> GetShader() const override { return m_BaseMaterial->GetShader(); }
		virtual void Bind() const override;

		const std::string& GetName() const { return m_Name; }
		Ref<Material> GetBaseMaterial() const { return m_BaseMaterial; }

		// Parameter Setters
		void SetFloat(const std::string& name, float value);
		void SetFloat2(const std::string& name, const glm::vec2& value);
		void SetFloat3(const std::string& name, const glm::vec3& value);
		void SetFloat4(const std::string& name, const glm::vec4& value);
		void SetTexture(const std::string& name, const Ref<Texture2D>& texture);

		static Ref<MaterialInstance> Create(const Ref<Material>& baseMaterial, const std::string& name = "MaterialInstance");

	private:
		Ref<Material> m_BaseMaterial;
		std::string m_Name;

		// Flat parameter tables (No Editor metadata here)
		std::unordered_map<std::string, float> m_Floats;
		std::unordered_map<std::string, glm::vec2> m_Float2s;
		std::unordered_map<std::string, glm::vec3> m_Float3s;
		std::unordered_map<std::string, glm::vec4> m_Float4s;
		std::unordered_map<std::string, Ref<Texture2D>> m_Textures;
	};

}
