#include <Quentlam.h>
#include "Sandbox3D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox3D::Sandbox3D()
	: Layer("Sandbox3D"), m_CameraController(45.0f, 1280.0f / 720.0f)
{
}

void Sandbox3D::OnAttach()
{
	QL_PROFILE_FUNCTION();
	m_CheckerboardTexture = Quentlam::Texture2D::Create("assets/texture/1.jpg"); // Use an existing texture

	m_Model = Quentlam::CreateRef<Quentlam::Model>("assets/models/spider.obj");
}

void Sandbox3D::OnDetach()
{
	QL_PROFILE_FUNCTION();
}

void Sandbox3D::OnEvent(Quentlam::Event& event)
{
	m_CameraController.OnEvent(event);
}

void Sandbox3D::OnUpdate(Quentlam::Timestep ts)
{
	QL_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);

	Quentlam::Renderer3D::ResetStats();
	
	Quentlam::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Quentlam::RenderCommand::Clear();

	Quentlam::Renderer3D::BeginScene(m_CameraController.GetCamera());

	// Draw some cubes
	Quentlam::Renderer3D::DrawCube(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.8f), { 0.8f, 0.2f, 0.3f, 1.0f });
	Quentlam::Renderer3D::DrawCube(glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.5f), m_CubeColor);

	// Draw imported model
	if (m_Model)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -2.0f)) 
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.05f)); // Scale down the spider model as it might be large
		Quentlam::Renderer3D::DrawModel(transform, *m_Model);
	}

	Quentlam::Renderer3D::EndScene();
}

void Sandbox3D::OnImGuiLayer()
{
	QL_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = Quentlam::Renderer3D::GetStatistics();
	ImGui::Text("Renderer3D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Cubes: %d", stats.CubeCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Cube Color", glm::value_ptr(m_CubeColor));

	ImGui::End();
}
