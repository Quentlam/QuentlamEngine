#pragma once
#include "Quentlam/Core/Timestep.h"
#include "Quentlam/Renderer/Texture.h"
#include "Quentlam/Scene/Scene.h"
#include "Quentlam/Scene/Entity.h"
#include "Random.h"
#include "ParticleSystem.h"

class Player
{
public:
	Player();
	~Player();

	void Init(Quentlam::Ref<Quentlam::Scene> scene);
	void LoadAssets();
	
	void OnUpdate(Quentlam::Timestep ts);
	void OnRender();
	void OnImGuiRenderer();

	void Reset();
	void UpdateParticles(Quentlam::Timestep ts) { m_ParticleSystem.OnUpdate(ts); }
	
	glm::vec2 GetPosition();
	float GetRotation();

	uint32_t GetScore() { return (uint32_t)((GetPosition().x + 10.0f) / 10.0f); }

	Quentlam::Entity GetEntity() { return m_Entity; }

	// Unify visual and collision transforms
	glm::vec2 m_VisualOffset = { 0.0f, 0.0f };
	glm::vec2 m_VisualScale = { 1.3f, 1.0f }; // Ship points +X, so width > height

private:
	Quentlam::Ref<Quentlam::Scene> m_Scene;
	Quentlam::Entity m_Entity;

	float m_Time = 0.0f;
	float m_SmokeEmitInterval = 0.4f;
	float m_SmokeNextEmitTime = m_SmokeEmitInterval;

	ParticleProps m_SmokeParticle, m_EngineParticle;
	ParticleSystem m_ParticleSystem;

	Quentlam::Ref<Quentlam::Texture2D> m_ShipTexture;
};

