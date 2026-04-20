#pragma once

#include "Quentlam/Core/Layer.h"
#include "Quentlam/Renderer/OrthographicCamera.h"
#include "Level.h"
#include "imgui/imgui.h"
#include "Quentlam/Events/MouseEvent.h"
#include "Quentlam/Events/ApplicationEvent.h"
#include "Quentlam/Events/Event.h"

struct GameState
{
	static GameState& Get() { static GameState instance; return instance; }
	uint32_t Score = 0;
	bool IsGameOver = false;
};

class GameScoreEvent : public Quentlam::Event
{
public:
	GameScoreEvent(uint32_t score) : m_Score(score) {}
	uint32_t GetScore() const { return m_Score; }

	static Quentlam::EventType GetStaticType() { return Quentlam::EventType::AppTick; }
	virtual Quentlam::EventType GetEventType() const override { return GetStaticType(); }
	virtual const char* GetName() const override { return "GameScoreEvent"; }
	virtual int GetCategoryFlags() const override { return Quentlam::EventCategoryApplication; }

private:
	uint32_t m_Score;
};

class GameLayer : public Quentlam::Layer
{
public:
	GameLayer();
	~GameLayer();
	void OnDetach()override;
	void OnAttach()override;
	
	void OnUpdate(Quentlam::Timestep ts)override;
	void OnImGuiLayer() override;
	void OnEvent(Quentlam::Event& event)override;
	bool OnMouseButtonPressed(Quentlam::MouseButtonPressedEvent& e);
	bool OnWindowsResize(Quentlam::WindowResizeEvent& e);

	void CreateCamera(uint32_t width, uint32_t height);

private:
	Quentlam::Scope<Quentlam::OrthographicCamera> m_Camera;
	Level m_Level;
	ImFont* m_Font;	
	float m_Time = 0.0f;
	bool m_Blink = false;

	enum class PlayState
	{
		Play = 0, MainMenu = 1, GameOver = 2
	};

	PlayState m_State = PlayState::MainMenu;
};



