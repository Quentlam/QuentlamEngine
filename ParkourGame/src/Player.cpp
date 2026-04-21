#include "Player.h"

#include "Quentlam/Core/Input.h"
#include "Quentlam/Core/KeyCodes.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include <box2d/box2d.h>

#ifndef QL_KEY_SPACE
#define QL_KEY_SPACE Quentlam::Key::SPACE
#endif

Player::Player()
{
	m_SmokeParticle.Position = { 0.0f,0.0f };
	m_SmokeParticle.Velocity = { 2.0f,0.0f }, m_SmokeParticle.VelocityVariation = { 4.0f,2.0f };
	m_SmokeParticle.SizeBegin = 0.35f, m_SmokeParticle.SizeEnd = 0.0f, m_SmokeParticle.SizeVariation = 0.15f;
	m_SmokeParticle.ColorBegin = { 0.8f,0.8f,0.8f,1.0f };
	m_SmokeParticle.ColorEnd = { 0.6f,0.6f,0.6f,1.0f };
	m_SmokeParticle.LifeTime = 4.0f;

	m_EngineParticle.Position = { 0.0f,0.0f };
	m_EngineParticle.Velocity = { -2.0f,0.0f }, m_EngineParticle.VelocityVariation = { 3.0f,1.0f };
	m_EngineParticle.SizeBegin = 0.5f, m_EngineParticle.SizeEnd = 0.0f, m_EngineParticle.SizeVariation = 0.3f;
	m_EngineParticle.ColorBegin = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f,1.0f };
	m_EngineParticle.ColorEnd = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f,1.0f };
	m_EngineParticle.LifeTime = 1.0f;
}

void Player::Init(b2World* world)
{
	m_World = world;
	Reset();
}

void Player::LoadAssets()
{
	// Load as 32-bit RGBA and GL_NEAREST. The engine's Texture2D::Create doesn't currently take filter parameters easily.
	// Assuming the engine has some way or we just use default.
	// Actually, we'll just use Create and hope it supports it, or modify texture creation later.
	m_ShipTexture = Quentlam::Texture2D::Create("assets/texture/Ship.png");
}

void Player::OnUpdate(Quentlam::Timestep ts)
{
	m_Time += ts;

	if (!m_Body) return;

	if (Quentlam::Input::IsKeyPressed(QL_KEY_SPACE))
	{
		b2Vec2 vel = m_Body->GetLinearVelocity();
		vel.y += m_EnginePower * ts * 60.0f; // Make it frame-rate independent
		if (vel.y < 0.0f)
			vel.y += m_EnginePower * ts * 60.0f * 2.0f;
		
		m_Body->SetLinearVelocity(vel);
		
		// Flames
		glm::vec2 emissionPoint = { 0.0f,-0.6f };
		float rotation = glm::radians(GetRotation());
		glm::vec4 rotated = glm::rotate(glm::mat4(1.0f), rotation, {0.0f,0.0f,1.0f}) * glm::vec4(emissionPoint,0.0f,1.0f);
		m_EngineParticle.Position = m_Position + glm::vec2{ rotated.x , rotated.y };
		m_EngineParticle.Velocity.y = -vel.y * 0.2f - 0.2f;
		m_ParticleSystem.Emit(m_EngineParticle);
	}

	b2Vec2 vel = m_Body->GetLinearVelocity();
	vel.y = glm::clamp(vel.y, -20.0f, 20.0f);
	m_Body->SetLinearVelocity(vel);

	// 3.1 Ship rotation based on velocity
	if (glm::length(glm::vec2(vel.x, vel.y)) > 0.01f)
	{
		glm::vec2 velDir = glm::normalize(glm::vec2(vel.x, vel.y));
		glm::vec2 upDir = { 0.0f, 1.0f };
		
		float targetAngle = glm::acos(glm::dot(upDir, velDir));
		if (vel.x > 0.0f) targetAngle = -targetAngle;

		// Smoothly interpolate the visual rotation independently of the physics body rotation
		float currentAngle = m_VisualRotation;
		float angleDiff = targetAngle - currentAngle;
		
		// Normalize angleDiff to [-PI, PI]
		while (angleDiff > glm::pi<float>()) angleDiff -= 2.0f * glm::pi<float>();
		while (angleDiff < -glm::pi<float>()) angleDiff += 2.0f * glm::pi<float>();

		// Calculate visual rotation
		float absDiff = glm::abs(glm::degrees(angleDiff));
		float rotationSpeed = (absDiff > 1.0f) ? glm::radians(300.0f) : glm::radians(120.0f);
		
		float step = rotationSpeed * (float)ts;
		if (glm::abs(angleDiff) < step)
			currentAngle = targetAngle;
		else
			currentAngle += glm::sign(angleDiff) * step;

		m_VisualRotation = currentAngle;
		
		// Set the physics body angle immediately to the target angle so physics doesn't jitter,
		// but keep visual rotation smooth.
		m_Body->SetTransform(m_Body->GetPosition(), targetAngle);
	}
	
	m_Position = { m_Body->GetPosition().x, m_Body->GetPosition().y };

	//Particles
	if (m_Time > m_SmokeNextEmitTime)
	{
		m_SmokeParticle.Position = m_Position;
		m_ParticleSystem.Emit(m_SmokeParticle);
		m_SmokeNextEmitTime += m_SmokeEmitInterval;
	}

	m_ParticleSystem.OnUpdate(ts);
}

void Player::OnImGuiRenderer()
{
}

void Player::Reset()
{
	m_Position = { -5.0f , 0.0f };
	m_Velocity = { 5.0f , 0.0f };
	m_Time = 0.0f;
	m_VisualRotation = 0.0f;
	m_SmokeNextEmitTime = m_SmokeEmitInterval;

	if (m_World)
	{
		if (m_Body)
		{
			m_World->DestroyBody(m_Body);
			m_Body = nullptr;
		}

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(m_Position.x, m_Position.y);
		bodyDef.linearDamping = 0.1f;
		bodyDef.angularDamping = 0.3f;
		bodyDef.gravityScale = 4.0f; // Make the player fall even faster
		bodyDef.bullet = true; // CCD enabled

		// Custom user data
		static int playerType = 1; // 1 for player
		bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(&playerType);

		m_Body = m_World->CreateBody(&bodyDef);

		b2PolygonShape dynamicBox;
		// Collision shape must precisely match visual triangle using b2PolygonShape
		// Visually it's a ship. The texture size is probably around 1.0f, 1.3f
		b2Vec2 vertices[3];
		// CCW order: top, bottom-left, bottom-right
		vertices[0].Set(0.0f, 0.65f);
		vertices[1].Set(-0.5f, -0.65f);
		vertices[2].Set(0.5f, -0.65f);
		dynamicBox.Set(vertices, 3);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.density = 1.0f; // Mass = 1.0 kg depending on area, but we can set mass data later
		fixtureDef.friction = 0.3f;

		m_Body->CreateFixture(&fixtureDef);
		
		b2MassData massData;
		m_Body->GetMassData(&massData);
		massData.mass = 1.0f; // Force mass to 1.0kg
		m_Body->SetMassData(&massData);

		m_Body->SetLinearVelocity(b2Vec2(5.0f, 0.0f));
	}
}

glm::vec2 Player::GetPosition() const
{
	return m_Position;
}

float Player::GetRotation() const
{
	return glm::degrees(m_VisualRotation);
}

void Player::OnRender()
{
	m_ParticleSystem.OnRender();
	Quentlam::Renderer2D::DrawRotatedQuad({ m_Position.x,m_Position.y,0.5f }, { 1.0f,1.3f }, m_ShipTexture, GetRotation());
}



Player::~Player()
{

 
}

