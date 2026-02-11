#pragma once

#include "Quentlam/Core/Layer.h"
#include "Platform/OpenGL/OpenGLShader.h"



class Sandbox2D : public Quentlam::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() = default;
    void OnAttach() override;
    void OnDetach() override;
	void OnEvent(Quentlam::Event& event) override;
	void OnUpdate(Quentlam::Timestep ts) override;
	void OnImGuiLayer() override;



private:
	Quentlam::OrthographicCameraController  m_CameraController;

	//Temp
	Quentlam::Ref<Quentlam::Texture2D>		m_Texture2D;
	Quentlam::Ref<Quentlam::VertexArray>	m_VertexArray;
	Quentlam::Ref<Quentlam::Shader>         m_FlatColorShader;

	glm::vec4 m_Square_Color{ 0.3f, 0.3f, 0.8f,1.0f };

};

