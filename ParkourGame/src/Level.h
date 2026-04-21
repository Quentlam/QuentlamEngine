#pragma once
#include "qlpch.h"
#include "Player.h"
#include "Quentlam/Core/Timestep.h"
#include "Pillar.h"
#include <box2d/box2d.h>

class Level : public b2ContactListener
{
public:
	void Init();

	void OnUpdate(Quentlam::Timestep ts);
	void OnRender();

	void OnImGuiRender();

	bool IsGameOver() const { return m_Gameover; }
	void Reset();

	Player& GetPlayer() { return m_Player; }

	// b2ContactListener interface
	virtual void BeginContact(b2Contact* contact) override;

	static void RunCollisionTests();

private:
	void CreatePillar(int index, float offset);
	bool CollisionTest();

	void GameOver();

private:
	Player m_Player;

	bool m_Gameover = false;
	
	float m_PillarsTarget = 30.0f;
	int m_PillarsIndex = 0;
	glm::vec3 m_PillarHSV = { 0.0f,0.8f,0.8f };

	std::vector<Pillar> m_Pillars;
	
	Quentlam::Ref<Quentlam::Texture2D> m_TriangleTexture;

	b2World* m_PhysicsWorld = nullptr;
	b2Body* m_TopBoundary = nullptr;
	b2Body* m_BottomBoundary = nullptr;
	bool m_ShowColliders = false;

	float m_TimeStep = 1.0f / 60.0f;
	float m_Accumulator = 0.0f;

	// Configuration for pillars
	static constexpr float MIN_GAP = 5.5f; // Increased for better playability
	static constexpr float MAX_GAP = 12.0f;
};

