#pragma once

#include "Quentlam/Core/Layer.h"
#include "Platform/OpenGL/OpenGLShader.h"


namespace Quentlam
{

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer() = default;
		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& event) override;
		void OnUpdate(Timestep ts) override;
		void OnImGuiLayer() override;



	private:
		OrthographicCameraController  m_CameraController;

		//Temp
		Ref<Texture2D>		m_Texture2D;
		Ref<Texture2D>		m_CheckerboardTexture;
		Ref<Texture2D>		m_SpriteSheet;
		Ref<SubTexture2D>	m_TextureStairs;
		Ref<SubTexture2D>	m_TextureBarrel;
		Ref<SubTexture2D>	m_TextureTree;

		Ref<FrameBuffer>	m_Framebuffer;
		glm::vec2 m_ViewportSize{ 0.0f,0.0f };

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		Ref<VertexArray>	m_VertexArray;
		Ref<Shader>         m_FlatColorShader;

		Entity m_SquareEntity;


		Ref<Scene>	m_ActiveScene;

		glm::vec4 m_Square_Color{ 0.3f, 0.3f, 0.8f,1.0f };

	};

}