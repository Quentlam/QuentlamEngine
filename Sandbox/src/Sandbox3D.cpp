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

	static float time = 0.0f;
	time += ts;

	// 1. Draw floor
	glm::mat4 floorTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 0.1f, 20.0f));
	Quentlam::Renderer3D::DrawCube(floorTransform, m_CheckerboardTexture, 10.0f);

	// 2. Draw central floating/rotating cube
	glm::mat4 centralTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f + glm::sin(time) * 0.5f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), time, glm::vec3(1.0f, 1.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
	Quentlam::Renderer3D::DrawCube(centralTransform, m_CubeColor);

	// 3. Draw a beautiful spiral of orbiting cubes
	for (int i = 0; i < 60; i++)
	{
		float offset = i * 0.15f;
		float angle = time * 1.5f + offset * 3.14159f;
		float radius = 2.0f + offset * 0.1f;
		float y = 0.0f + offset * 0.15f + glm::sin(time + offset) * 0.5f;
		
		glm::vec3 pos(glm::cos(angle) * radius, y, glm::sin(angle) * radius);
		
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::rotate(glm::mat4(1.0f), time * 3.0f + offset, glm::vec3(0.5f, 1.0f, 0.2f))
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.4f - (i * 0.005f))); // gradually get smaller

		glm::vec4 color(
			(glm::sin(offset + time * 1.0f) + 1.0f) * 0.5f,
			(glm::cos(offset + time * 1.2f) + 1.0f) * 0.5f,
			(glm::sin(offset + time * 0.8f) + 1.0f) * 0.5f,
			1.0f
		);

		Quentlam::Renderer3D::DrawCube(transform, color);
	}

	// 4. Draw imported model if loaded
	if (m_Model)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -2.0f)) 
			* glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
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
