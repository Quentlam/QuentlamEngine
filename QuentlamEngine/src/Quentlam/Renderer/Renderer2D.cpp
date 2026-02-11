#include "qlpch.h"
#include "Renderer2D.h"


#include "VertexArray.h"
#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Quentlam
{
	struct Renderer2DStorge
	{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
	};

	static Renderer2DStorge* s_Data;

	void Renderer2D::Init()
	{
		QL_PROFILE_FUNCTION();

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


		s_Data->WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData,sizeof(uint32_t));
		
		
		s_Data->TextureShader = Shader::Create("assets/shaders/Texture2DShader.glsl");
		s_Data->TextureShader->SetInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		QL_PROFILE_FUNCTION();

		delete s_Data;
	}
	void Renderer2D::BeginScene(OrthographicCamera& camera)
	{
		QL_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

	}
	void Renderer2D::EndScene()
	{
		QL_PROFILE_FUNCTION();


	}
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x,position.y,0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		QL_PROFILE_FUNCTION();

		s_Data->TextureShader->SetFloat4("u_Color", color);
		//Bind white texture here
		s_Data->WhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0)); //偏移到设置的位置去，并且设置为设置的大小。
		s_Data->TextureShader->SetMat4("u_Transform", transform);

		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tinColor)
	{
		DrawQuad({ position.x,position.y,0.0f }, size, texture, tilingFactor, tinColor);
	}


	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tinColor)
	{
		QL_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0)); //偏移到设置的位置去，并且设置为设置的大小。
		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Color", tinColor);
		s_Data->TextureShader->SetFloat("m_TilingFactor", tilingFactor);


		texture->Bind();
		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const glm::vec4& color)
	{
		QL_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		glm::mat4 transform =
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f,0.0f,1.0f })
			* glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0)); //先旋转到旋转的位置去，再偏移到设置的位置去，最后并且设置为设置的大小。
		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Color", color);


		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}



	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float rotation, const float tilingFactor, const glm::vec4& tinColor)
	{
		DrawRotatedQuad(position, size, texture, rotation, tilingFactor, tinColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float rotation, const float tilingFactor, const glm::vec4& tinColor)
	{
		QL_PROFILE_FUNCTION();

		s_Data->TextureShader->Bind();
		glm::mat4 transform =
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f,0.0f,1.0f })
			* glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0)); //先旋转到旋转的位置去，再偏移到设置的位置去，最后并且设置为设置的大小。
		s_Data->TextureShader->SetMat4("u_Transform", transform);
		s_Data->TextureShader->SetFloat4("u_Color", tinColor);
		s_Data->TextureShader->SetFloat("m_TilingFactor", tilingFactor);


		texture->Bind();
		s_Data->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}



}

