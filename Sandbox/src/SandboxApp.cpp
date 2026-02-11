#include <Quentlam.h>
//----------Entry Point --------------------------
#include <Quentlam/Core/EntryPoint.h>



#include "Platform/OpenGL/OpenGLShader.h"



#include "imgui/imgui.h"


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Sandbox2D.h"


class ExampleLayer : public Quentlam::Layer {
public:
	ExampleLayer()
	{

	}

	void OnUpdate(Quentlam::Timestep ts) override
	{

	}

	void OnEvent(Quentlam::Event& event) override
	{

	}

	void OnImGuiLayer() override
	{

	}


private:

};

class Sandbox :public Quentlam::Application
{  
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox()
	{

	}
};

Quentlam::Application* Quentlam::CreateApplication()
{
	return new Sandbox();
}

