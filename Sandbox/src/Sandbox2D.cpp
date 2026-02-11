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

	m_Texture2D = Quentlam::Texture2D::Create("assets/texture/child.jpg");
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
	m_CameraController.OnUpdate(ts);

	//Renderer
	{
		QL_PROFILE_SCOPE("Renderer Prep");
		Quentlam::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Quentlam::RenderCommand::Clear();
	}

	{
		QL_PROFILE_SCOPE("Renderer Draw");
		Quentlam::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Quentlam::Renderer2D::DrawQuad({ -1.0f,0.0f }, { 0.8f,0.8f }, m_Square_Color);
		Quentlam::Renderer2D::DrawQuad({ 0.5f,-0.5f }, { 0.5f,0.75f }, { 0.2f,0.3f,0.8f,1.0f });
		Quentlam::Renderer2D::DrawRotatedQuad({ 0.2f,0.5f,-0.05f }, { 5.0f,5.0f }, glm::radians(30.0f), { 0.7f,0.5f,0.8f,1.0f });
		Quentlam::Renderer2D::DrawRotatedQuad({ 0.2f,0.5f,-0.1f }, { 10.0f,10.0f }, m_Texture2D, glm::radians(45.0f), 10.0f, { 0.7f,0.5f,0.8f,1.0f });
	}

	Quentlam::Renderer2D::EndScene();

}

void Sandbox2D::OnImGuiLayer()
{
	QL_PROFILE_FUNCTION();
	ImGui::Begin("Test");
	ImGui::Text("Just for Test");
	ImGui::ColorEdit4("Square color", glm::value_ptr(m_Square_Color));
	ImGui::End();
}
