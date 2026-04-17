#include "qlpch.h"
#include "Player.h"

#include "Quentlam/Core/Input.h"
#include "Quentlam/Core/KeyCodes.h"
#include "Quentlam/Core/MouseButtonCodes.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include "Quentlam/Scene/Components.h"
#include "DataManager.h"
#include "PrefabManager.h"
#include <imgui.h>

#include <Box2D/include/box2d/b2_body.h>

Player::Player()
{
	m_SmokeParticle.Position = { 0.0f,0.0f };
	m_SmokeParticle.Velocity = { -2.0f,0.0f }, m_SmokeParticle.VelocityVariation = { 4.0f,2.0f };
	m_SmokeParticle.SizeBegin = 0.35f, m_SmokeParticle.SizeEnd = 0.0f, m_SmokeParticle.SizeVariation = 0.15f;
	m_SmokeParticle.ColorBegin = { 0.8f,0.8f,0.8f,1.0f };
	m_SmokeParticle.ColorEnd = { 0.6f,0.6f,0.6f,1.0f };
	m_SmokeParticle.LifeTime = 4.0f;

	m_EngineParticle.Position = { 0.0f,0.0f };
	m_EngineParticle.Velocity = { -2.0f,0.0f }, m_EngineParticle.VelocityVariation = { 3.0f,1.0f };
	m_EngineParticle.SizeBegin = 0.5f, m_EngineParticle.SizeEnd = 0.0f, m_EngineParticle.SizeVariation = 0.3f;
	m_EngineParticle.LifeTime = 1.0f;
}

Player::~Player()
{
}

void Player::Init(Quentlam::Ref<Quentlam::Scene> scene)
{
	m_Scene = scene;
	m_Entity = PrefabManager::InstantiatePlayer(m_Scene);

	auto& pConfig = DataManager::GetConfig().Player;
	m_EngineParticle.ColorBegin = pConfig.EngineParticleColorBegin;
	m_EngineParticle.ColorEnd = pConfig.EngineParticleColorEnd;
}

void Player::LoadAssets()
{
	m_ShipTexture = PrefabManager::LoadAddressableTexture("Ship");
}

void Player::OnUpdate(Quentlam::Timestep ts)
{
	m_Time += ts;

	if (!m_Entity) return;

	auto& rb = m_Entity.GetComponent<Quentlam::Rigidbody2DComponent>();
	if (rb.RuntimeBody)
	{
		b2Body* body = (b2Body*)rb.RuntimeBody;
		b2Vec2 vel = body->GetLinearVelocity();
		
		auto& pConfig = DataManager::GetConfig().Player;
		
		// Constant forward velocity
		uint32_t gapsPassed = GetScore();
		float speedMultiplier = 1.0f;
		if (gapsPassed >= DataManager::GetConfig().Level.ConsecutiveGapsForEvent)
		{
			speedMultiplier = DataManager::GetConfig().Level.EventSpeedMultiplier;
		}
		vel.x = pConfig.InitialVelocity.x * speedMultiplier;

		bool isThrusting = Quentlam::Input::IsKeyPressed(Quentlam::Key::Space);
		
		// Only check mouse button if ImGui doesn't want to capture it
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			isThrusting |= Quentlam::Input::IsMouseButtonPressed(Quentlam::Mouse::ButtonLeft);
		}

		if (isThrusting)
		{
			vel.y += 0.5f * ts * pConfig.EnginePower; // Engine power (like old m_EnginePower)
			if (vel.y < 0.0f)
				vel.y += 1.0f * ts * pConfig.EnginePower; // Recover faster from falling (m_EnginePower * 2.0f)
		}
		else
		{
			vel.y -= 0.4f * ts * pConfig.Gravity; // Gravity (like old m_Gravity)
		}

		vel.y = glm::clamp(vel.y, -pConfig.MaxVelocityY, pConfig.MaxVelocityY);
		body->SetLinearVelocity(vel);

		// Update rotation based on velocity vector
		// Since the base texture points +X (it is horizontal), we don't need to subtract 90 degrees anymore.
		// Note: We need to explicitly calculate the rotation and translation
		// to update the TransformComponent so that rendering and collision polygon match exactly.
		float targetAngle = std::atan2(vel.y, vel.x);
		body->SetTransform(body->GetPosition(), targetAngle);
		
		auto& tc = m_Entity.GetComponent<Quentlam::TransformComponent>();
		tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(body->GetPosition().x, body->GetPosition().y, 0.0f)) * 
					   glm::rotate(glm::mat4(1.0f), targetAngle, glm::vec3(0.0f, 0.0f, 1.0f)) *
					   glm::scale(glm::mat4(1.0f), glm::vec3(m_VisualScale.x, m_VisualScale.y, 1.0f)); // Unified scale

		// Particle System emission
		glm::vec2 pos = GetPosition();
		
		// The emission point is at the back of the ship texture (local -X).
		glm::vec2 emissionPoint = { -0.6f, 0.0f };
		// Rotate the local emission point by the current ship angle to get world space offset
		glm::vec4 rotated = glm::rotate(glm::mat4(1.0f), targetAngle, {0.0f, 0.0f, 1.0f}) * glm::vec4(emissionPoint, 0.0f, 1.0f);
		m_EngineParticle.Position = pos + glm::vec2{ rotated.x, rotated.y };
		
		// Particle velocity should be opposite to ship movement direction, plus some downward push from the engine
		glm::vec2 particleDir = -glm::normalize(glm::vec2(vel.x, vel.y));
		m_EngineParticle.Velocity = particleDir * 3.0f; // Shoot backwards with speed 3.0


		if (isThrusting)
		{
			m_EngineParticle.ColorBegin = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f }; // Orange
			m_EngineParticle.ColorEnd = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };   // Yellow
			m_EngineParticle.SizeBegin = 0.5f;
			m_ParticleSystem.Emit(m_EngineParticle);
			// Emit a second particle for thicker trail
			m_EngineParticle.SizeBegin = 0.4f;
			m_ParticleSystem.Emit(m_EngineParticle);
		}
		else
		{
			// Free fall / Idle smoke particles
			m_EngineParticle.ColorBegin = { 0.6f, 0.6f, 0.6f, 0.8f }; // Gray
			m_EngineParticle.ColorEnd = { 0.3f, 0.3f, 0.3f, 0.2f };   // Dark gray
			m_EngineParticle.SizeBegin = 0.25f;
			m_ParticleSystem.Emit(m_EngineParticle);
		}
	}

	// Particles
	if (m_Time > m_SmokeNextEmitTime)
	{
		m_SmokeParticle.Position = GetPosition();
		m_ParticleSystem.Emit(m_SmokeParticle);
		m_SmokeNextEmitTime += m_SmokeEmitInterval;
	}

	m_ParticleSystem.OnUpdate(ts);
}

void Player::OnImGuiRenderer()
{
	ImGui::Begin("Player Transform Debug");
	ImGui::DragFloat2("Visual Offset", glm::value_ptr(m_VisualOffset), 0.01f);
	ImGui::DragFloat2("Visual Scale", glm::value_ptr(m_VisualScale), 0.01f);
	
	glm::vec2 pos = GetPosition();
	float rot = GetRotation();
	ImGui::Text("Logic Pos: (%.3f, %.3f)", pos.x, pos.y);
	ImGui::Text("Logic Rot: %.3f rad (%.1f deg)", rot, glm::degrees(rot));
	
	glm::vec4 rotatedOffset = glm::rotate(glm::mat4(1.0f), rot, {0.0f, 0.0f, 1.0f}) * glm::vec4(m_VisualOffset, 0.0f, 1.0f);
	glm::vec2 spritePos = pos + glm::vec2(rotatedOffset);
	ImGui::Text("Sprite World Pos: (%.3f, %.3f)", spritePos.x, spritePos.y);
	
	ImGui::End();
}

void Player::Reset()
{
	m_Time = 0.0f;
	m_SmokeNextEmitTime = m_SmokeEmitInterval;

	if (m_Entity)
	{
		auto& tc = m_Entity.GetComponent<Quentlam::TransformComponent>();
		tc.Transform = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(m_VisualScale.x, m_VisualScale.y, 1.0f));

		auto& rb = m_Entity.GetComponent<Quentlam::Rigidbody2DComponent>();
		if (rb.RuntimeBody)
		{
			auto& pConfig = DataManager::GetConfig().Player;
			b2Body* body = (b2Body*)rb.RuntimeBody;
			body->SetTransform(b2Vec2(-5.0f, 0.0f), 0.0f);
			body->SetLinearVelocity(b2Vec2(pConfig.InitialVelocity.x, pConfig.InitialVelocity.y));
			body->SetAngularVelocity(0.0f);
		}
	}
}

void Player::OnRender()
{
	m_ParticleSystem.OnRender();
	
	if (m_Entity)
	{
		glm::vec2 pos = GetPosition();
		float rot = GetRotation();

		// Calculate world space offset by rotating local offset
		glm::vec4 rotatedOffset = glm::rotate(glm::mat4(1.0f), rot, {0.0f, 0.0f, 1.0f}) * glm::vec4(m_VisualOffset, 0.0f, 1.0f);
		glm::vec3 finalPos = { pos.x + rotatedOffset.x, pos.y + rotatedOffset.y, 0.5f };

		// Construct Transform Matrix for DrawSprite
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), finalPos)
							* glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0.0f, 0.0f, 1.0f))
							* glm::scale(glm::mat4(1.0f), glm::vec3(m_VisualScale.x, m_VisualScale.y, 1.0f));

		Quentlam::Renderer2D::DrawSprite(transform, m_ShipTexture, 1.0f, glm::vec4(1.0f));
	}
}

glm::vec2 Player::GetPosition()
{
	if (!m_Entity) return { 0.0f, 0.0f };
	auto& tc = m_Entity.GetComponent<Quentlam::TransformComponent>();
	return { tc.Transform[3].x, tc.Transform[3].y };
}

float Player::GetRotation()
{
	if (!m_Entity) return 0.0f;
	auto& tc = m_Entity.GetComponent<Quentlam::TransformComponent>();
	return atan2(tc.Transform[0][1], tc.Transform[0][0]);
}

