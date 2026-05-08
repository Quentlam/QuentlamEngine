#include <Quentlam.h>
#include "GameLayer.h"
#include "Quentlam/Core/Application.h"
#include "Quentlam/Events/Event.h"
#include "Quentlam/Renderer/RenderCommand.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include "Quentlam/Core/Input.h"
#include "Quentlam/Core/KeyCodes.h"
#include "ParkourSystem.h"
#include "ParkourComponents.h"

#include <box2d/b2_body.h>


GameLayer::GameLayer()
	:Quentlam::Layer("GameLayer")
{
	auto& window = Quentlam::Application::Get().GetWindow();
	CreateCamera(window.GetWidth(), window.GetHeight());
}

GameLayer::~GameLayer()
{
}

void GameLayer::OnAttach()
{
	Quentlam::RenderCommand::SetClearColor({ 0.1f,0.1f,0.1f,1 });
	CreateCamera(Quentlam::Application::Get().GetWindow().GetWidth(), Quentlam::Application::Get().GetWindow().GetHeight());
	
	m_ActiveScene = Quentlam::CreateRef<Quentlam::Scene>();
	if (!Quentlam::ParkourSystem::LoadScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene"))
	{
		if (!Quentlam::ParkourSystem::LoadScene(m_ActiveScene.get(), "../QL-Editor/assets/configs/ParkourTemplate.scene"))
		{
			QL_CORE_WARN("Failed to load ParkourTemplate.scene, falling back to default build.");
			Quentlam::ParkourSystem::BuildScene(m_ActiveScene.get());
		}
	}
	m_ActiveScene->OnRuntimeStart();
	Quentlam::ParkourSystem::OnRuntimeStart(m_ActiveScene.get());

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

	bool isPaused = false;
	if (m_ActiveScene)
	{
		auto uiView = m_ActiveScene->GetRegistry().view<Quentlam::UIFlowComponent>();
		for (auto entity : uiView)
		{
			if (uiView.get<Quentlam::UIFlowComponent>(entity).CurrentState == Quentlam::UIFlowComponent::State::Paused)
			{
				isPaused = true;
				break;
			}
		}
	}

	float gameTs = isPaused ? 0.0f : (float)ts;
	Quentlam::ParkourSystem::OnUpdate(m_ActiveScene.get(), gameTs);

	auto playerView = m_ActiveScene->GetRegistry().view<Quentlam::PlayerControllerComponent, Quentlam::TransformComponent>();
	for (auto entity : playerView)
	{
		auto& tc = playerView.get<Quentlam::TransformComponent>(entity);
		m_Camera->SetPosition({ tc.Transform[3].x, tc.Transform[3].y, 0.0f });
		break;
	}

	// Render
	Quentlam::RenderCommand::SetClearColor({ 0.0f,0.0f,0.0f ,1.0f});
	Quentlam::RenderCommand::Clear();

	Quentlam::Renderer2D::BeginScene(*m_Camera);
	
	m_ActiveScene->OnUpdateRuntime(gameTs);
	
	// Particle render logic is integrated into the ParkourSystem or can be handled here
	if (m_ActiveScene)
	{
		auto particleView = m_ActiveScene->GetRegistry().view<Quentlam::SimpleParticleSystemComponent>();
		for (auto entity : particleView)
		{
			auto& psc = particleView.get<Quentlam::SimpleParticleSystemComponent>(entity);
			for (auto& particle : psc.ParticlePool)
			{
				if (!particle.Active) continue;
				float life = particle.LifeRemaining / particle.LifeTime;
				glm::vec4 color = glm::mix(particle.ColorEnd, particle.ColorBegin, life);
				color.a = color.a * life;
				float size = glm::mix(particle.SizeEnd, particle.SizeBegin, life);
				Quentlam::Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);
			}
		}
	}
	
	Quentlam::Renderer2D::EndScene();
}

void GameLayer::OnImGuiLayer()
{
	bool isPlaying = false;
	bool isGameOver = false;
	bool isMainMenu = false;
	bool isPaused = false;
	uint32_t playerScore = 0;

	auto uiView = m_ActiveScene->GetRegistry().view<Quentlam::UIFlowComponent, Quentlam::ScoreSystemComponent>();
	for (auto entity : uiView)
	{
		auto [ui, score] = uiView.get<Quentlam::UIFlowComponent, Quentlam::ScoreSystemComponent>(entity);
		playerScore = score.CurrentScore;
		if (ui.CurrentState == Quentlam::UIFlowComponent::State::Playing) isPlaying = true;
		if (ui.CurrentState == Quentlam::UIFlowComponent::State::GameOver) isGameOver = true;
		if (ui.CurrentState == Quentlam::UIFlowComponent::State::MainMenu) isMainMenu = true;
		if (ui.CurrentState == Quentlam::UIFlowComponent::State::Paused) isPaused = true;
	}

	if (isPlaying)
	{
		std::string scoreStr = std::string("Score: ") + std::to_string(playerScore);
		ImGui::GetForegroundDrawList()->AddText(m_Font, 48.0f, ImGui::GetWindowPos(), 0xffffffff, scoreStr.c_str());
	}
	else if (isPaused)
	{
		auto pos = ImGui::GetWindowPos();
		auto width = Quentlam::Application::Get().GetWindow().GetWidth();
		
		std::string scoreStr = std::string("Score: ") + std::to_string(playerScore);
		ImGui::GetForegroundDrawList()->AddText(m_Font, 48.0f, pos, 0xffffffff, scoreStr.c_str());
		
		pos.x += width * 0.5f - 200.0f;
		pos.y += 100.0f;
		if (m_Blink)
			ImGui::GetForegroundDrawList()->AddText(m_Font, 120.0f, pos, 0xffffffff, "PAUSED");
	}
	else if (isGameOver)
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
		std::string scoreStr = std::string("Score: ") + std::to_string(playerScore);
		ImGui::GetForegroundDrawList()->AddText(m_Font, 48.0f, pos, 0xffffffff, scoreStr.c_str());
	}
	else if (isMainMenu)
	{
		auto pos = ImGui::GetWindowPos();
		auto width = Quentlam::Application::Get().GetWindow().GetWidth();
		pos.x += width * 0.5f - 300.0f;
		pos.y += 50.0f;
		if (m_Blink)
			ImGui::GetForegroundDrawList()->AddText(m_Font, 120.0f, pos, 0xffffffff, "Click to start!");
	}
}

void GameLayer::OnDetach()
{
	Quentlam::ParkourSystem::OnRuntimeStop(m_ActiveScene.get());
	m_ActiveScene->OnRuntimeStop();
}

void GameLayer::OnEvent(Quentlam::Event& e)
{
	Quentlam::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Quentlam::WindowResizeEvent>(QL_BIND_EVENT_FN(GameLayer::OnWindowsResize));
	dispatcher.Dispatch<Quentlam::MouseButtonPressedEvent>(QL_BIND_EVENT_FN(GameLayer::OnMouseButtonPressed));
	dispatcher.Dispatch<Quentlam::KeyPressedEvent>(QL_BIND_EVENT_FN(GameLayer::OnKeyPressed));
}

bool GameLayer::OnMouseButtonPressed(Quentlam::MouseButtonPressedEvent& e)
{
	auto uiView = m_ActiveScene->GetRegistry().view<Quentlam::UIFlowComponent>();
	for (auto entity : uiView)
	{
		auto& ui = uiView.get<Quentlam::UIFlowComponent>(entity);
		if (ui.CurrentState == Quentlam::UIFlowComponent::State::GameOver)
		{
			Quentlam::ParkourSystem::OnRuntimeStop(m_ActiveScene.get());
			m_ActiveScene->OnRuntimeStop();
			m_ActiveScene = Quentlam::CreateRef<Quentlam::Scene>();
			
			// Try to load the preset instead of just building the default
			if (!Quentlam::ParkourSystem::LoadScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene"))
			{
				if (!Quentlam::ParkourSystem::LoadScene(m_ActiveScene.get(), "../QL-Editor/assets/configs/ParkourTemplate.scene"))
				{
					Quentlam::ParkourSystem::BuildScene(m_ActiveScene.get());
				}
			}
			
			m_ActiveScene->OnRuntimeStart();
			Quentlam::ParkourSystem::OnRuntimeStart(m_ActiveScene.get());
			break; // Break because m_ActiveScene has been recreated
		}
		else if (ui.CurrentState == Quentlam::UIFlowComponent::State::MainMenu)
		{
			ui.CurrentState = Quentlam::UIFlowComponent::State::Playing;

			auto playerView = m_ActiveScene->GetRegistry().view<Quentlam::PlayerControllerComponent, Quentlam::Rigidbody2DComponent>();
			for (auto [pEntity, player, rb] : playerView.each())
			{
				if (rb.RuntimeBody)
				{
					b2Body* body = (b2Body*)rb.RuntimeBody;
					body->SetAwake(true);
					b2Vec2 vel = body->GetLinearVelocity();
					vel.y = player.EnginePower * 10.0f; // Boost!
					body->SetLinearVelocity(vel);
				}
			}
			
			auto obstacleView = m_ActiveScene->GetRegistry().view<Quentlam::TagComponent, Quentlam::Rigidbody2DComponent>();
			for (auto [e, tag, obsRb] : obstacleView.each())
			{
				if (tag.Tag.find("Pillar") == 0 && obsRb.RuntimeBody)
				{
					b2Body* obsBody = (b2Body*)obsRb.RuntimeBody;
					obsBody->SetAwake(true); // Wake up obstacles
				}
			}
			
			break;
		}
	}
	return false;
}

bool GameLayer::OnKeyPressed(Quentlam::KeyPressedEvent& e)
{
	auto uiView = m_ActiveScene->GetRegistry().view<Quentlam::UIFlowComponent>();
	for (auto entity : uiView)
	{
		auto& ui = uiView.get<Quentlam::UIFlowComponent>(entity);
		if (e.GetKeyCode() == Quentlam::Key::SPACE)
		{
			if (ui.CurrentState == Quentlam::UIFlowComponent::State::GameOver)
			{
				Quentlam::ParkourSystem::OnRuntimeStop(m_ActiveScene.get());
				m_ActiveScene->OnRuntimeStop();
				m_ActiveScene = Quentlam::CreateRef<Quentlam::Scene>();
				
				if (!Quentlam::ParkourSystem::LoadScene(m_ActiveScene.get(), "assets/configs/ParkourTemplate.scene"))
				{
					if (!Quentlam::ParkourSystem::LoadScene(m_ActiveScene.get(), "../QL-Editor/assets/configs/ParkourTemplate.scene"))
					{
						Quentlam::ParkourSystem::BuildScene(m_ActiveScene.get());
					}
				}
				
				m_ActiveScene->OnRuntimeStart();
				Quentlam::ParkourSystem::OnRuntimeStart(m_ActiveScene.get());
				break;
			}
			else if (ui.CurrentState == Quentlam::UIFlowComponent::State::MainMenu)
			{
				ui.CurrentState = Quentlam::UIFlowComponent::State::Playing;

				auto playerView = m_ActiveScene->GetRegistry().view<Quentlam::PlayerControllerComponent, Quentlam::Rigidbody2DComponent>();
				for (auto [pEntity, player, rb] : playerView.each())
				{
					if (rb.RuntimeBody)
					{
						b2Body* body = (b2Body*)rb.RuntimeBody;
						body->SetAwake(true);
						b2Vec2 vel = body->GetLinearVelocity();
						vel.y = player.EnginePower * 10.0f; // Boost!
						body->SetLinearVelocity(vel);
					}
				}
				
				auto obstacleView = m_ActiveScene->GetRegistry().view<Quentlam::TagComponent, Quentlam::Rigidbody2DComponent>();
				for (auto [e, tag, obsRb] : obstacleView.each())
				{
					if (tag.Tag.find("Pillar") == 0 && obsRb.RuntimeBody)
					{
						b2Body* obsBody = (b2Body*)obsRb.RuntimeBody;
						obsBody->SetAwake(true); // Wake up obstacles
					}
				}
				
				break;
			}
		}
		else if (e.GetKeyCode() == Quentlam::Key::Escape)
		{
			if (ui.CurrentState == Quentlam::UIFlowComponent::State::Playing)
			{
				ui.CurrentState = Quentlam::UIFlowComponent::State::Paused;
			}
			else if (ui.CurrentState == Quentlam::UIFlowComponent::State::Paused)
			{
				ui.CurrentState = Quentlam::UIFlowComponent::State::Playing;
				
				// Restore velocities
				auto playerView = m_ActiveScene->GetRegistry().view<Quentlam::PlayerControllerComponent, Quentlam::Rigidbody2DComponent>();
				for (auto [pEntity, player, rb] : playerView.each())
				{
					if (rb.RuntimeBody)
					{
						b2Body* body = (b2Body*)rb.RuntimeBody;
						body->SetAwake(true); // Wake up body
						b2Vec2 vel = body->GetLinearVelocity();
						vel.y = player.EnginePower * 5.0f; // Small boost on unpause
						body->SetLinearVelocity(vel);
					}
				}
				
				auto obstacleView = m_ActiveScene->GetRegistry().view<Quentlam::TagComponent, Quentlam::Rigidbody2DComponent>();
				for (auto [e, tag, obsRb] : obstacleView.each())
				{
					if (tag.Tag.find("Pillar") == 0 && obsRb.RuntimeBody)
					{
						b2Body* obsBody = (b2Body*)obsRb.RuntimeBody;
						obsBody->SetAwake(true); // Wake up obstacles
					}
				}
			}
			break;
		}
	}
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








