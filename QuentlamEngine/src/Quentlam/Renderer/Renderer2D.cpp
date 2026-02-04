#include "qlpch.h"
#include "Renderer2D.h"


#include "VertexArray.h"
#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"


namespace Quentlam
{
	struct Renderer2DStorge
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;

	};

	static Renderer2DStorge* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DStorge();

		s_Data->QuadVertexArray = VertexArray::Create();
		float vertices[3 * 7] = {
	   -0.5f, 0.5f,0.0f,0.0f,1.0f,
	   -0.5f,-0.5f,0.0f,0.0f,0.0f,
		0.5f,-0.5f,0.0f,1.0f,0.0f,
		0.5f, 0.5f,0.0f,1.0f,1.0f
		};

		Ref<VertexBuffer>vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		vertexBuffer->SetLayout({
			{ShaderDataType::Float3,"u_Position"},
			{ ShaderDataType::Float2,"TexCoord" }
			});
		s_Data->QuadVertexArray->AddVertexBuffer(vertexBuffer);


		std::vector<uint32_t> indices =
		{
		  0,1,2,2,3,0
		};
		Ref<IndexBuffer>indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices.data(), (uint32_t)indices.size()));
		s_Data->QuadVertexArray->SetIndexBuffer(indexBuffer);

		//m_Texture2D = Texture2D::Create("assets/texture/child.jpg");
		s_Data->FlatColorShader = Shader::Create("assets/shaders/FlatColorShader.glsl");

	}
	void Renderer2D::Shutdown()
	{
		delete s_Data;

	}
	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_Transform", glm::mat4(1.0f));

	}
	void Renderer2D::EndScene()
	{


	}
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x,position.y,0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Quentlam::OpenGLShader>(s_Data->FlatColorShader)->UploadUniformFloat4("u_Color", color);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}



}

