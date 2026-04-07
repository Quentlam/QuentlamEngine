#include <Quentlam.h>
#include "Sandbox2D.h"
#include "Quentlam/Events/ApplicationEvent.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"

#include "Quentlam/Renderer/VertexArray.h"
#include <chrono>

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{

}

void Sandbox2D::OnAttach()
{
	QL_PROFILE_FUNCTION();

	m_Texture2D = Quentlam::Texture2D::Create("assets/texture/background.png");
	m_SpriteSheet = Quentlam::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");
	m_TextureStairs = Quentlam::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7,6 }, { 128,128 });
	m_TextureBarrel = Quentlam::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8,2 }, { 128,128 });
	m_TextureTree = Quentlam::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2,1 }, { 128,128 }, { 1,2 });

}

void Sandbox2D::OnDetach()
{
	QL_PROFILE_FUNCTION();


}

void Sandbox2D::OnEvent(Quentlam::Event& event)
{
	m_CameraController.OnEvent(event);

	if (event.GetEventType() == Quentlam::EventType::WindowResize)
	{
		auto& re = (Quentlam::WindowResizeEvent&)event;

		float zoom = (float)re.GetWidth() / 1280.0f;
		m_CameraController.SetZoomLevel(zoom);
	}
}

void Sandbox2D::OnUpdate(Quentlam::Timestep ts)
{
	QL_PROFILE_FUNCTION();
	Quentlam::Renderer2D::ResetStats();

	m_CameraController.OnUpdate(ts);

	//Renderer
	{
		QL_PROFILE_SCOPE("Renderer Prep");
		Quentlam::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Quentlam::RenderCommand::Clear();
	}


#if 0

	{
		QL_PROFILE_SCOPE("Renderer Draw");
		static float rotation = 20.0f;
		rotation += ts * 20.0f;


		Quentlam::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Quentlam::Renderer2D::DrawQuad({ -1.5f,0.0f }, { 0.8f,0.8f }, m_Square_Color);
		Quentlam::Renderer2D::DrawQuad({ -1.0f,0.0f }, { 0.8f,0.8f }, { 0.8f,0.2f,0.3f,1.0f });//红色
		Quentlam::Renderer2D::DrawQuad({ 0.5f,-0.5f }, { 0.5f,0.75f }, { 0.2f,0.3f,0.8f,1.0f });//蓝色
		Quentlam::Renderer2D::DrawQuad({ 0.0f,0.0f, -0.1f }, { 10.0f,10.0f }, m_Texture2D, 10.0f, { 0.7f,0.5f,0.8f,1.0f });//background
		Quentlam::Renderer2D::DrawRotatedQuad({ 0.2f,0.5f }, { 1.0f,1.0f }, 30.0f, { 0.7f,0.5f,0.8f,1.0f });//紫色30°旋转
		Quentlam::Renderer2D::DrawRotatedQuad({ 0.0f,0.0f }, { 1.0f,1.0f }, m_Texture2D, rotation, 1.0f, { 0.7f,0.5f,0.8f,1.0f });//小棋盘
		Quentlam::Renderer2D::EndScene();
	}

#endif


	Quentlam::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Quentlam::Renderer2D::DrawQuad({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f }, m_TextureStairs);
	Quentlam::Renderer2D::DrawQuad({ 1.0f,0.0f,0.0f }, { 1.0f,1.0f }, m_TextureBarrel);
	Quentlam::Renderer2D::DrawQuad({ -1.0f,0.0f,0.0f }, { 1.0f,2.0f }, m_TextureTree);

	Quentlam::Renderer2D::EndScene();


}

void Sandbox2D::OnImGuiLayer()
{
	QL_PROFILE_FUNCTION();
	ImGui::Begin("Test");

	auto stats = Quentlam::Renderer2D::GetStatistics();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());


	ImGui::ColorEdit4("Square color", glm::value_ptr(m_Square_Color));
	ImGui::End();
}
