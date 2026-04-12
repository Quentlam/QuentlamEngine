#pragma once

#include "Quentlam/Base/Layer.h"
#include "Platform/OpenGL/OpenGLShader.h"



class EditorLayer : public Quentlam::Layer
{
public:
	EditorLayer();
	~EditorLayer() = default;
    void OnAttach() override;
    void OnDetach() override;
	void OnEvent(Quentlam::Event& event) override;
	void OnUpdate(Quentlam::Timestep ts) override;
	void OnImGuiLayer() override;



private:
	Quentlam::OrthographicCameraController  m_CameraController;

	//Temp
	Quentlam::Ref<Quentlam::Texture2D>		m_Texture2D;
	Quentlam::Ref<Quentlam::Texture2D>		m_CheckerboardTexture;
	Quentlam::Ref<Quentlam::Texture2D>		m_SpriteSheet;
	Quentlam::Ref<Quentlam::SubTexture2D>	m_TextureStairs;
	Quentlam::Ref<Quentlam::SubTexture2D>	m_TextureBarrel;
	Quentlam::Ref<Quentlam::SubTexture2D>	m_TextureTree;

	Quentlam::Ref<Quentlam::FrameBuffer>	m_Framebuffer;
	glm::vec2 m_ViewportSize{ 0.0f,0.0f };

	bool m_ViewportFocused = false, m_ViewportHovered = false;

	Quentlam::Ref<Quentlam::VertexArray>	m_VertexArray;
	Quentlam::Ref<Quentlam::Shader>         m_FlatColorShader;

	glm::vec4 m_Square_Color{ 0.3f, 0.3f, 0.8f,1.0f };

};

