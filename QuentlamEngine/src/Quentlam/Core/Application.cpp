#include "qlpch.h"

#include "Input.h"
#include "Application.h"
#include "../Renderer/Renderer.h"

#include <glfw/glfw3.h>

namespace Quentlam
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;


	Application::Application()
	{
		QL_CORE_ASSERTS(!s_Instance, "Application already exists! ");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();




		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

	}
	Application::~Application()
	{

	}
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::OnEvent(Event& e)//
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));//分发器分发WindowCloseEvent事件--判断窗口是否关闭
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));//分发器分发WindowResizeEvent的事件--判断窗口是大小有变化

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
			break;
		}
	}

	void Application::Run()
	{ 
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;


			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)layer->OnUpdate(timestep);
			}
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)layer->OnImGuiLayer();
			m_ImGuiLayer->End();




			m_Window->OnUpdate();
		};
	}


	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}


	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			QL_CORE_WARN("m_Minimized : {0}", m_Minimized);
			return false;
		}
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());


		m_Minimized = false;
		QL_CORE_WARN("m_Minimized : {0}", m_Minimized);
		return false;
	}
}