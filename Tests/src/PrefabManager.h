#pragma once
#include "Quentlam/Scene/Scene.h"
#include "Quentlam/Scene/Entity.h"
#include "Quentlam/Scene/Components.h"
#include "Quentlam/Renderer/Texture.h"
#include "DataManager.h"
#include <string>
#include <unordered_map>

class PrefabManager
{
public:
	static void Init()
	{
		s_Instance = new PrefabManager();
		// In a real addressable system, this would load asset catalogs
		// We simulate "Addressables" by caching textures
		auto& pConfig = DataManager::GetConfig().Player;
		s_Instance->m_Textures["Ship"] = Quentlam::Texture2D::Create(pConfig.ShipTexturePath);
		s_Instance->m_Textures["Triangle"] = Quentlam::Texture2D::Create("assets/texture/Triangle.png");
	}

	static void Shutdown()
	{
		delete s_Instance;
		s_Instance = nullptr;
	}

	static Quentlam::Entity InstantiatePillarTop(Quentlam::Ref<Quentlam::Scene> scene, const std::string& name)
	{
		Quentlam::Entity entity = scene->CreateEntity(name);
		entity.AddComponent<Quentlam::SpriteTransformComponent>();
		auto& rb = entity.AddComponent<Quentlam::Rigidbody2DComponent>();
		rb.Type = Quentlam::Rigidbody2DComponent::BodyType::Static;
		auto& pc = entity.AddComponent<Quentlam::PolygonCollider2DComponent>();
		// Triangle points: Top tip (0, 0.5), Bottom Left (-0.5, -0.5), Bottom Right (0.5, -0.5)
		pc.Vertices = {
			{ 0.0f, 0.5f },
			{ 0.5f, -0.5f },
			{ -0.5f, -0.5f }
		};
		return entity;
	}

	static Quentlam::Entity InstantiatePillarBottom(Quentlam::Ref<Quentlam::Scene> scene, const std::string& name)
	{
		Quentlam::Entity entity = scene->CreateEntity(name);
		entity.AddComponent<Quentlam::SpriteTransformComponent>();
		auto& rb = entity.AddComponent<Quentlam::Rigidbody2DComponent>();
		rb.Type = Quentlam::Rigidbody2DComponent::BodyType::Static;
		auto& pc = entity.AddComponent<Quentlam::PolygonCollider2DComponent>();
		pc.Vertices = {
			{ 0.0f, 0.5f },
			{ -0.5f, -0.5f },
			{ 0.5f, -0.5f }
		};
		return entity;
	}

	static Quentlam::Entity InstantiatePlayer(Quentlam::Ref<Quentlam::Scene> scene)
	{
		Quentlam::Entity entity = scene->CreateEntity("Player");
		auto& tc = entity.GetComponent<Quentlam::TransformComponent>();
		// Since ship texture points +X, scale is {1.3f, 1.0f} (wider than tall)
		tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.3f, 1.0f, 1.0f));

		entity.AddComponent<Quentlam::SpriteTransformComponent>(glm::vec4(1.0f));
		
		auto& rb = entity.AddComponent<Quentlam::Rigidbody2DComponent>();
		rb.Type = Quentlam::Rigidbody2DComponent::BodyType::Dynamic;
		rb.FixedRotation = false; // Allow rotation
		rb.InitialLinearVelocity = DataManager::GetConfig().Player.InitialVelocity;

		auto& bc = entity.AddComponent<Quentlam::BoxCollider2DComponent>();
		// Since we now apply scale in TransformComponent, the collider size should be normalized (1.0x1.0 space)
		// BoxCollider2DComponent.Size is half-extents, so we divide by 2
		bc.Size = glm::vec2(0.5f, 0.5f);
		bc.Density = 1.0f;
		bc.Friction = 0.0f;
		bc.Restitution = 0.0f;
		return entity;
	}

	static Quentlam::Ref<Quentlam::Texture2D> LoadAddressableTexture(const std::string& key)
	{
		if (s_Instance->m_Textures.find(key) != s_Instance->m_Textures.end())
			return s_Instance->m_Textures[key];
		return nullptr;
	}

private:
	std::unordered_map<std::string, Quentlam::Ref<Quentlam::Texture2D>> m_Textures;
	static PrefabManager* s_Instance;
};