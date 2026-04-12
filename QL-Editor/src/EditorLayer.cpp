#include <Quentlam.h>
#include "EditorLayer.h"
#include "Quentlam/Events/ApplicationEvent.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"

#include "Quentlam/Renderer/VertexArray.h"
#include <chrono>

EditorLayer::EditorLayer()
	:Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f)
{

}

void EditorLayer::OnAttach()
{
	QL_PROFILE_FUNCTION();

	m_Texture2D = Quentlam::Texture2D::Create("assets/texture/background.png");
	m_SpriteSheet = Quentlam::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");
	m_TextureStairs = Quentlam::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7,6 }, { 128,128 });
	m_TextureBarrel = Quentlam::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8,2 }, { 128,128 });
	m_TextureTree = Quentlam::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2,1 }, { 128,128 }, { 1,2 });

	Quentlam::FrameBufferSpecification fbSpec;
	fbSpec.Width = 1280;
	fbSpec.Height = 720;


	m_Framebuffer = Quentlam::FrameBuffer::Create(fbSpec);
}

void EditorLayer::OnDetach()
{
	QL_PROFILE_FUNCTION();


}

void EditorLayer::OnEvent(Quentlam::Event& event)
{
	m_CameraController.OnEvent(event);

	if (event.GetEventType() == Quentlam::EventType::WindowResize)
	{
		auto& re = (Quentlam::WindowResizeEvent&)event;

		float zoom = (float)re.GetWidth() / 1280.0f;
		m_CameraController.SetZoomLevel(zoom);
	}
}

void EditorLayer::OnUpdate(Quentlam::Timestep ts)
{
	QL_PROFILE_FUNCTION();
	Quentlam::Renderer2D::ResetStats();
	

	if(m_ViewportFocused)
	m_CameraController.OnUpdate(ts);

	//Renderer
	{
		QL_PROFILE_SCOPE("Renderer Prep");
		m_Framebuffer->Bind();
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
		Quentlam::Renderer2D::DrawQuad({ -1.0f,0.0f }, { 0.8f,0.8f }, { 0.8f,0.2f,0.3f,1.0f });//��ɫ
		Quentlam::Renderer2D::DrawQuad({ 0.5f,-0.5f }, { 0.5f,0.75f }, { 0.2f,0.3f,0.8f,1.0f });//��ɫ
		Quentlam::Renderer2D::DrawQuad({ 0.0f,0.0f, -0.1f }, { 10.0f,10.0f }, m_Texture2D, 10.0f, { 0.7f,0.5f,0.8f,1.0f });//background
		Quentlam::Renderer2D::DrawRotatedQuad({ 0.2f,0.5f }, { 1.0f,1.0f }, 30.0f, { 0.7f,0.5f,0.8f,1.0f });//��ɫ30����ת
		Quentlam::Renderer2D::DrawRotatedQuad({ 0.0f,0.0f }, { 1.0f,1.0f }, m_Texture2D, rotation, 1.0f, { 0.7f,0.5f,0.8f,1.0f });//С����
		Quentlam::Renderer2D::EndScene();
	}

#endif


	Quentlam::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Quentlam::Renderer2D::DrawQuad({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f }, m_TextureStairs);
	Quentlam::Renderer2D::DrawQuad({ 1.0f,0.0f,0.0f }, { 1.0f,1.0f }, m_TextureBarrel);
	Quentlam::Renderer2D::DrawQuad({ -1.0f,0.0f,0.0f }, { 1.0f,2.0f }, m_TextureTree);

	Quentlam::Renderer2D::EndScene();
	m_Framebuffer->UnBind();

}

void EditorLayer::OnImGuiLayer()
{
	QL_PROFILE_FUNCTION();



	bool dockSpaceOpen = true;

	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}


	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;


	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
				Quentlam::Application::Get().Close();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}


	ImGui::Begin("Test");
	auto stats = Quentlam::Renderer2D::GetStatistics();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::ColorEdit4("Square color", glm::value_ptr(m_Square_Color));
	
	ImGui::End();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Viewport");


	m_ViewportFocused = ImGui::IsWindowFocused();
	m_ViewportHovered = ImGui::IsWindowHovered();
	Quentlam::Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);//当视口窗口被选中时，阻止事件传递到其他层



	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize))
	{
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);//调整帧缓冲区的大小
		m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);//调整摄像机的比例大小
	}

	uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
	ImGui::Image((void*)(intptr_t)textureID, ImVec2{ m_ViewportSize.x,m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
	ImGui::PopStyleVar();

	ImGui::End();




	ImGui::End();

}
