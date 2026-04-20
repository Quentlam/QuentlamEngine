#pragma once

#include "Quentlam/Core/Timestep.h"
#include "Quentlam/Renderer/Texture.h"
#include "Random.h"
#include "ParticleSystem.h"
#include "Color.h"

class b2World;
class b2Body;

class Player
{
public:
	void Init(b2World* world);
	void LoadAssets();
	
	void OnUpdate(Quentlam::Timestep ts);
	void OnRender();

	void OnImGuiRenderer();

	void Reset();
	void UpdateParticles(Quentlam::Timestep ts) { m_ParticleSystem.OnUpdate(ts);};
	glm::vec2 GetPosition() const;


	float GetRotation() const;

	b2Body* GetBodyForTesting() const { return m_Body; }

	Quentlam::Ref<Quentlam::Texture2D> GetShipTexRef() { return m_ShipTexture; };

	Player();
	~Player();
private:
	glm::vec2 m_Position = { -5.0f , 0.0f };
	glm::vec2 m_Velocity = { 5.0f , 0.0f };

	float m_EnginePower = 1.0f;

	float m_Time = 0.0f;
	float m_SmokeEmitInterval = 0.4f;
	float m_SmokeNextEmitTime = m_SmokeEmitInterval;

	ParticleProps m_SmokeParticle, m_EngineParticle;
	ParticleSystem m_ParticleSystem;

	Quentlam::Ref<Quentlam::Texture2D> m_ShipTexture;

	b2World* m_World = nullptr;
	b2Body* m_Body = nullptr;
	float m_VisualRotation = 0.0f;
};

