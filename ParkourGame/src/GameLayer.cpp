#include "GameLayer.h"
#include "Quentlam/Core/Application.h"
#include "Quentlam/Events/Event.h"
#include "Quentlam/Renderer/RenderCommand.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include "Random.h"

GameLayer::GameLayer()
	:Quentlam::Layer("GameLayer")
{
	auto& window = Quentlam::Application::Get().GetWindow();
	CreateCamera(window.GetWidth(), window.GetHeight());

	Random::Init();
}


GameLayer::~GameLayer()
{


}
void GameLayer::OnAttach()
{
	Quentlam::RenderCommand::SetClearColor({ 0.1f,0.1f,0.1f,1 });
	CreateCamera(Quentlam::Application::Get().GetWindow().GetWidth(), Quentlam::Application::Get().GetWindow().GetHeight());
	
	Quentlam::Renderer2D::Init();
	m_Level.Init();

	ImGuiIO io = ImGui::GetIO();
	std::string fontPath = "assets/fonts/opensans/OpenSans-Regular.ttf";
	std::ifstream fontFile(fontPath);
	if (!fontFile.good())
	{
		fontPath = "ParkourGame/assets/fonts/opensans/OpenSans-Regular.ttf";
		fontFile.open(fontPath);
	}
	if (!fontFile.good())
	{
		fontPath = "../ParkourGame/assets/fonts/opensans/OpenSans-Regular.ttf";
		fontFile.open(fontPath);
	}

	if (fontFile.good())
	{
		m_Font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 120.0f);
	}
	else
	{
		QL_CORE_ERROR("Failed to load font: {0}", fontPath);
		m_Font = io.Fonts->AddFontDefault();
	}
}
void GameLayer::OnUpdate(Quentlam::Timestep ts)
{
	m_Time += ts;
	if ((int)(m_Time * 10.0f) % 8 > 4)
		m_Blink = !m_Blink;

	if (m_Level.IsGameOver())
	{
		m_State = PlayState::GameOver;
		GameState::Get().IsGameOver = true;
	}

	const auto& playerPos = m_Level.GetPlayer().GetPosition();
	m_Camera->SetPosition({ playerPos.x,playerPos.y,0.0f });

	switch (m_State)
	{
		case PlayState::Play:
		{
			m_Level.OnUpdate(ts);
			break;
		}
	}

	m_Level.GetPlayer().UpdateParticles(ts);

	// Render
	Quentlam::RenderCommand::SetClearColor({ 0.0f,0.0f,0.0f ,1.0f});
	Quentlam::RenderCommand::Clear();

	Quentlam::Renderer2D::BeginScene(*m_Camera);
	m_Level.OnRender();
	Quentlam::Renderer2D::EndScene();
}
void GameLayer::OnImGuiLayer()
{
	switch (m_State)
	{
		case PlayState::Play:
		{
			uint32_t playerScore = GameState::Get().Score;
			std::string scoreStr = std::string("Score: ") + std::to_string(playerScore);
			ImGui::GetForegroundDrawList()->AddText(m_Font, 48.0f, ImGui::GetWindowPos(), 0xffffffff, scoreStr.c_str());
			break;
		}
		case PlayState::MainMenu:
		{
			auto pos = ImGui::GetWindowPos();
			auto width = Quentlam::Application::Get().GetWindow().GetWidth();
			auto heigt = Quentlam::Application::Get().GetWindow().GetHeight();
			pos.x += width * 0.5f - 300.0f;
			pos.y += 50.0f;
			if(m_Blink)
				ImGui::GetForegroundDrawList()->AddText(m_Font, 120.0f, pos, 0xffffffff, "Click to play!");
			break;
		}
		case PlayState::GameOver:
		{
			auto pos = ImGui::GetWindowPos();
			auto width = Quentlam::Application::Get().GetWindow().GetWidth();
			auto heigt = Quentlam::Application::Get().GetWindow().GetHeight();
			pos.x += width * 0.5f - 300.0f;
			pos.y += 50.0f;
			if (m_Blink)
				ImGui::GetForegroundDrawList()->AddText(m_Font, 120.0f, pos, 0xffffffff, "Click to play!");

			pos.x += 200.0f;
			pos.y += 150.0f;
			uint32_t playerScore = GameState::Get().Score;
			std::string scoreStr = std::string("Score: ") + std::to_string(playerScore);
			ImGui::GetForegroundDrawList()->AddText(m_Font, 48.0f, pos, 0xffffffff, scoreStr.c_str());
			break;
		}
	}
	
	m_Level.OnImGuiRender();
}
void GameLayer::OnDetach()
{


}


void GameLayer::OnEvent(Quentlam::Event& e)
{
	Quentlam::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Quentlam::WindowResizeEvent>(QL_BIND_EVENT_FN(GameLayer::OnWindowsResize));
	dispatcher.Dispatch<Quentlam::MouseButtonPressedEvent>(QL_BIND_EVENT_FN(GameLayer::OnMouseButtonPressed));

}

bool GameLayer::OnMouseButtonPressed(Quentlam::MouseButtonPressedEvent& e)
{
	if (m_State == PlayState::GameOver)
		m_Level.Reset();

	m_State = PlayState::Play;
	GameState::Get().IsGameOver = false;
	return false;
}


bool GameLayer::OnWindowsResize(Quentlam::WindowResizeEvent& e)
{
	CreateCamera(e.GetWidth(), e.GetHeight());
	return false;
}

void GameLayer::CreateCamera(uint32_t width, uint32_t height)
{
	float aspectRatio = (float)width / (float)height;

	float camWidth = 8.0f;
	float bottom = -camWidth;
	float top = camWidth;
	float left = bottom * aspectRatio;
	float right = top * aspectRatio;
	m_Camera = Quentlam::CreateScope<Quentlam::OrthographicCamera>(left, right, bottom, top);
}








