#include <Quentlam.h>
#include "Sandbox2D.h"
#include "Quentlam/Events/ApplicationEvent.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"

#include "Quentlam/Renderer/VertexArray.h"

Sandbox2D::Sandbox2D()
	:Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f)
{

}

void Sandbox2D::OnAttach()
{

}

void Sandbox2D::OnDetach()
{


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
	//Camera
	m_CameraController.OnUpdate(ts);

	//Renderer
	Quentlam::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
	Quentlam::RenderCommand::Clear();



	Quentlam::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Quentlam::Renderer2D::DrawQuad({ 0.0f,0.0f }, { 1.0f,1.0f }, m_Square_Color);
	Quentlam::Renderer2D::EndScene();
	
	//TODO:
	//std::dynamic_pointer_cast<Quentlam::OpenGLShader>(m_FlatColorShader)->Bind();
	//std::dynamic_pointer_cast<Quentlam::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_Square_Color);

}

void Sandbox2D::OnImGuiLayer()
{
	ImGui::Begin("Test");
	ImGui::Text("Just for Test");
	ImGui::ColorEdit4("Square color", glm::value_ptr(m_Square_Color));
	ImGui::End();
}
