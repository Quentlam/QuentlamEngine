#include "qlpch.h"
#include "Material.h"

namespace Quentlam {

	// ==========================================
	// Material Implementation
	// ==========================================

	Material::Material(const Ref<Shader>& shader, const std::string& name)
		: m_Shader(shader), m_Name(name)
	{
	}

	void Material::Bind() const
	{
		if (m_Shader)
		{
			m_Shader->Bind();
			// Set default material parameters globally here
		}
	}

	Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name)
	{
		return CreateRef<Material>(shader, name);
	}

	// ==========================================
	// MaterialInstance Implementation
	// ==========================================

	MaterialInstance::MaterialInstance(const Ref<Material>& baseMaterial, const std::string& name)
		: m_BaseMaterial(baseMaterial), m_Name(name)
	{
	}

	void MaterialInstance::Bind() const
	{
		m_BaseMaterial->Bind();
		auto shader = m_BaseMaterial->GetShader();
		if (!shader) return;

		// Bind all flat parameter overrides
		// In a real engine, we'd hash names to Uniform Buffer offsets, but this is a placeholder.

		// e.g. for (auto& [name, value] : m_Floats) shader->SetFloat(name, value);
		// Note: we assume OpenGLShader handles these safely
	}

	void MaterialInstance::SetFloat(const std::string& name, float value)
	{
		m_Floats[name] = value;
	}

	void MaterialInstance::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		m_Float2s[name] = value;
	}

	void MaterialInstance::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		m_Float3s[name] = value;
	}

	void MaterialInstance::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		m_Float4s[name] = value;
	}

	void MaterialInstance::SetTexture(const std::string& name, const Ref<Texture2D>& texture)
	{
		m_Textures[name] = texture;
	}

	Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& baseMaterial, const std::string& name)
	{
		return CreateRef<MaterialInstance>(baseMaterial, name);
	}

}
