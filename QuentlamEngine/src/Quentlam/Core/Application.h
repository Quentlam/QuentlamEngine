#pragma once
#include "Core.h"
#include "../Events/KeyEvent.h"
#include "../Events/ApplicationEvent.h"

#include "Timestep.h"

#include "LayerStack.h"
#include "../Events/Event.h"
#include "../Events/MouseEvent.h"
#include "Window.h"
#include "../ImGui/ImGuiLayer.h"

#include "Quentlam/Renderer/VertexArray.h"
#include "Quentlam/Renderer/Shader.h"
#include "../Renderer/Buffer.h"


#include "../Renderer/OrthographicCamera.h"
namespace Quentlam
{
	class QUENTLAM_API Application
	{
	public:
		Application();
		virtual ~Application();
		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
		bool m_Running = true;
		bool m_Minimized = false;//是否为最小化的标志位

	private:
		static Application* s_Instance;
		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;
	};

	Application* CreateApplication();

}

