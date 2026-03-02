#pragma once

#include "Quentlam/Core/Layer.h"
#include "Quentlam/Renderer/OrthographicCamera.h"
#include "Level.h"
#include "imgui/imgui.h"
#include "Quentlam/Events/MouseEvent.h"
#include "Quentlam/Events/ApplicationEvent.h"


class GameLayer : public Quentlam::Layer
{
public:
	GameLayer();
	~GameLayer();
	void OnDetach()override;
	void OnAttach()override;
	
	void OnUpdate(Quentlam::Timestep ts)override;
	void OnImGuiRender() override;
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

	enum class GameState
	{
		Play = 0, MainMenu = 1, GameOver = 2
	};

	GameState m_State = GameState::MainMenu;
};



