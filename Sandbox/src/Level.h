#pragma once
#include "qlpch.h"
#include "Player.h"
#include "Quentlam/Core/Timestep.h"
#include "Pillar.h"

class Level
{
public:
	void Init();

	void OnUpdate(Quentlam::Timestep ts);
	void OnRender();

	void OnImGuiRender();

	bool IsGameOver() const { return m_Gameover; }
	void Reset();

	Player& GetPlayer() { return m_Player; }
private:
	void CreatePillar(int index, float offset);
	bool CollisionTest();

	void GameOver();

private:
	Player m_Player;

	bool m_Gameover = false;
	
	float m_PillarsTarget = 30.0f;
	int m_PillarsIndex = 0.0f;
	glm::vec3 m_PillarHSV = { 0.0f,0.8f,0.8f };

	std::vector<Pillar> m_Pillars;
	
	Quentlam::Ref<Quentlam::Texture2D> m_TriangleTexture;

};

