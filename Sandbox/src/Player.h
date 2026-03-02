#pragma once

#include "Quentlam/Core/Timestep.h"
#include "Quentlam/Renderer/Texture.h"
#include "Random.h"
#include "ParticleSystem.h"
#include "Color.h"


class Player
{
public:
	void LoadAssets();
	
	void OnUpdate(Quentlam::Timestep ts);
	void OnRender();

	void OnImGuiRenderer();

	void Reset();
	void UpdateParticles(Quentlam::Timestep ts) { m_ParticleSystem.OnUpdate(ts);};
	const glm::vec2& GetPosition()const { return m_Position; }


	float GetRotation() { return m_Velocity.y * 4.0f - 90.0f; }
	const glm::vec2& GetRotation() const { return m_Position; }

	Quentlam::Ref<Quentlam::Texture2D> GetShipTexRef() { return m_ShipTexture; };

	uint32_t GetScore() const { return (uint32_t)(m_Position.x + 10.0f ) / 10.0f ; }

	Player();
	~Player();
private:
	glm::vec2 m_Position = { -5.0f , 0.0f };
	glm::vec2 m_Velocity = { 5.0f , 0.0f };

	float m_EnginePower = 0.5f;
	float m_Gravity = 0.4f;

	float m_Time = 0.0f;
	float m_SmokeEmitInterval = 0.4f;
	float m_SmokeNextEmitTime = m_SmokeEmitInterval;

	ParticleProps m_SmokeParticle, m_EngineParticle;
	ParticleSystem m_ParticleSystem;

	Quentlam::Ref<Quentlam::Texture2D> m_ShipTexture;
};

