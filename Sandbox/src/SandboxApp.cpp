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
		: m_CameraController(1280.0f / 720.0f), Layer("Example")
	{
		m_VertexArray = Quentlam::VertexArray::Create();
		float vertices[3 * 7] = {
	   -0.5f,-0.5f,0.0f,  0.8f,0.2f,0.8f,1.0f,
		0.5f,-0.5f,0.0f,  0.2f,0.3f,0.8f,1.0f,
		0.0f, 0.5f,0.0f,  0.8f,0.8f,0.2f,1.0f
		};

		Quentlam::Ref<Quentlam::VertexBuffer>vertexBuffer;
		vertexBuffer.reset(Quentlam::VertexBuffer::Create(vertices, sizeof(vertices)));
		vertexBuffer->SetLayout({
			{Quentlam::ShaderDataType::Float3,"a_Position"},
			{Quentlam::ShaderDataType::Float4,"a_Color"}
			});
		m_VertexArray->AddVertexBuffer(vertexBuffer);


		std::vector<uint32_t> indices =
		{
		  0,1,2
		};
		Quentlam::Ref<Quentlam::IndexBuffer>indexBuffer;
		indexBuffer.reset(Quentlam::IndexBuffer::Create(indices.data(), (uint32_t)indices.size()));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		m_BlueVertexArray = Quentlam::VertexArray::Create();

		float BlueVertices[5 * 4] =
		{
			-0.5f, 0.5f,0.0f,0.0f,1.0f,
			-0.5f,-0.5f,0.0f,0.0f,0.0f,
			 0.5f,-0.5f,0.0f,1.0f,0.0f,
			 0.5f, 0.5f,0.0f,1.0f,1.0f
		};


		Quentlam::Ref<Quentlam::VertexBuffer> BlueVB;
		BlueVB.reset(Quentlam::VertexBuffer::Create(BlueVertices, sizeof(BlueVertices)));
		Quentlam::BufferLayout BlueLayout = {
			{ Quentlam::ShaderDataType::Float3,"b_Position" },
			{ Quentlam::ShaderDataType::Float2,"TexCoord" }
		};
		BlueVB->SetLayout(BlueLayout);


		std::vector<uint32_t> BlueIndices = { 0,1,2,2,3,0 };
		Quentlam::Ref<Quentlam::IndexBuffer> BlueIB;
		BlueIB.reset(Quentlam::IndexBuffer::Create(BlueIndices.data(), (uint32_t)BlueIndices.size()));


		m_BlueVertexArray->SetIndexBuffer(BlueIB);
		m_BlueVertexArray->AddVertexBuffer(BlueVB);


		m_Shader = m_ShaderLibrary.Load("Colorful Square shader", "assets/shaders/ColorChangeShader.glsl");
		m_BlueShader = m_ShaderLibrary.Load("Blue Square shader", "assets/shaders/BlueShader.glsl");
		m_Texture2DShader = m_ShaderLibrary.Load("Texture shader", "assets/shaders/Texture.glsl");



		m_Texture2D_child = Quentlam::Texture2D::Create("assets/texture/child.jpg");
		m_Texture2D_merlin = Quentlam::Texture2D::Create("assets/texture/Merlin.png");



		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(m_Texture2DShader)->Bind();
		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(m_Texture2DShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Quentlam::Timestep ts) override
	{
		m_CameraController.OnUpdate(ts);
		Quentlam::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
		Quentlam::RenderCommand::Clear();


		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(m_BlueShader)->Bind();
		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(m_BlueShader)->UploadUniformFloat4("u_Color", m_square_Color);


		if (Quentlam::Input::IsKeyPressed(QL_KEY_J))
		{
			m_SquarePosition.x -= m_SquareSpeed * ts;
		}
		if (Quentlam::Input::IsKeyPressed(QL_KEY_L))
		{
			m_SquarePosition.x += m_SquareSpeed * ts;
		}
		if (Quentlam::Input::IsKeyPressed(QL_KEY_K))
		{
			m_SquarePosition.y -= m_SquareSpeed * ts;
		}
		if (Quentlam::Input::IsKeyPressed(QL_KEY_I))
		{
			m_SquarePosition.y += m_SquareSpeed * ts;
		}

		Quentlam::Renderer::BeginScene(m_CameraController.GetCamera());
		m_CameraController.GetCamera();
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		glm::mat4 transfrom = glm::translate(glm::mat4(1.0f), m_SquarePosition);



		
		Quentlam::Renderer::Submit(m_Shader, m_VertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(m_trangle_scale)));

		Quentlam::Renderer::Submit(m_Shader, m_BlueVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(m_Square_scale)) * transfrom);

		Quentlam::Renderer::Submit(m_Shader, m_BlueVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(m_Square2_scale)));



		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				glm::vec3 pos(i * 0.11f, j * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Quentlam::Renderer::Submit(m_BlueShader, m_BlueVertexArray, transform);
			}
		}

		m_Texture2D_child->Bind();
		Quentlam::Renderer::Submit(m_Texture2DShader, m_BlueVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(m_child_scale)));


		m_Texture2D_merlin->Bind();
		Quentlam::Renderer::Submit(m_Texture2DShader, m_BlueVertexArray, glm::scale(glm::mat4(1.0f), glm::vec3(m_merlin_scale)));


		Quentlam::Renderer::EndScene();
	}

	void OnEvent(Quentlam::Event& event) override
	{
		m_CameraController.OnEvent(event);

		if (event.GetEventType() == Quentlam::EventType::WindowResize)
		{
			auto& re = (Quentlam::WindowResizeEvent&)event;

			float zoom = (float)re.GetWidth() / 1280.0f;
			m_CameraController.SetZoomLevel(zoom);
		}
	}

	void OnImGuiLayer() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Just for Test");
		ImGui::ColorEdit4("square color", glm::value_ptr(m_square_Color));
		ImGui::SliderFloat("trangle scale slider", &m_trangle_scale,0.1f,2.0f);
		ImGui::SliderFloat("square scale slider", &m_Square_scale, 0.1f, 2.0f);
		ImGui::SliderFloat("square2 scale slider", &m_Square2_scale, 0.1f, 2.0f);
		ImGui::SliderFloat("child scale slider", &m_child_scale, 0.1f, 2.0f);
		ImGui::SliderFloat("merlin scale slider", &m_merlin_scale, 0.1f, 2.0f);
		ImGui::End();
	}


private:
	Quentlam::ShaderLibrary					m_ShaderLibrary;
	Quentlam::Ref<Quentlam::Shader>			m_Shader;
	Quentlam::Ref<Quentlam::Shader>			m_BlueShader;
	Quentlam::Ref<Quentlam::Shader>			m_Texture2DShader;

	Quentlam::Ref<Quentlam::VertexArray>	m_VertexArray;
	Quentlam::Ref<Quentlam::VertexArray>	m_BlueVertexArray;
	Quentlam::Ref<Quentlam::Texture2D>		m_Texture2D_child;
	Quentlam::Ref<Quentlam::Texture2D>		m_Texture2D_merlin;

	Quentlam::OrthographicCameraController  m_CameraController;

	glm::vec3 m_SquarePosition				= { 0.0f ,0.0f,0.0f };
	glm::vec4 m_square_Color				{ 0.8f, 0.2f, 0.3f, 1.0f };
	glm::vec4 m_SecondLineColor				 { 0.3f, 0.2f, 0.8f, 1.0f };


	float m_trangle_scale					= 1.0f;
	float m_Square_scale					= 1.0f;
	float m_Square2_scale					= 1.0f;
	float m_child_scale						= 1.0f;
	float m_merlin_scale					= 1.0f;
	float m_SquareSpeed						= 5.0f;
	float m_CameraMoveSpeed					= 5.0f;
	float m_CameraRotation					= 0.0f;
	float m_CameraRotationSpeed				= 180.0f;


};

class Sandbox :public Quentlam::Application
{  
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());

	}
	~Sandbox()
	{

	}
};

Quentlam::Application* Quentlam::CreateApplication()
{
	return new Sandbox();
}

