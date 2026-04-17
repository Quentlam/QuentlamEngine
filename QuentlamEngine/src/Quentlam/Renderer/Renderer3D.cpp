#include "qlpch.h"
#include "Renderer3D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Quentlam
{
	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		int EntityID;
	};

	struct Renderer3DData
	{
		static const uint32_t MaxCubes = 1000;
		static const uint32_t MaxVertices = MaxCubes * 8; // 8 unique vertices per cube if flat shaded, but usually 24 for proper normals. For simplicity, we'll use 24 vertices for a cube (4 per face * 6 faces) to allow proper texture mapping per face. Let's use 24.
		static const uint32_t MaxIndices = MaxCubes * 36; // 6 faces * 2 triangles * 3 indices
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Shader> ModelShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr;
		CubeVertex* CubeVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 CubeVertexPositions[24];
		glm::vec2 CubeTexCoords[24];

		Renderer3D::Statistics Stats;
	};

	static Renderer3DData s_Data3D;

	void Renderer3D::Init()
	{
		QL_PROFILE_FUNCTION();

		s_Data3D.CubeVertexArray = VertexArray::Create();

		// For a cube, we need 24 vertices to have correct texture coordinates per face
		const uint32_t maxVertices = s_Data3D.MaxCubes * 24;
		s_Data3D.CubeVertexBuffer = VertexBuffer::Create(maxVertices * sizeof(CubeVertex));
		s_Data3D.CubeVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexIndex" },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_EntityID" }
		});
		s_Data3D.CubeVertexArray->AddVertexBuffer(s_Data3D.CubeVertexBuffer);

		s_Data3D.CubeVertexBufferBase = new CubeVertex[maxVertices];

		uint32_t* cubeIndices = new uint32_t[s_Data3D.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data3D.MaxIndices; i += 36)
		{
			for (int face = 0; face < 6; face++)
			{
				cubeIndices[i + face * 6 + 0] = offset + 0;
				cubeIndices[i + face * 6 + 1] = offset + 1;
				cubeIndices[i + face * 6 + 2] = offset + 2;

				cubeIndices[i + face * 6 + 3] = offset + 2;
				cubeIndices[i + face * 6 + 4] = offset + 3;
				cubeIndices[i + face * 6 + 5] = offset + 0;

				offset += 4;
			}
		}

		Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, s_Data3D.MaxIndices);
		s_Data3D.CubeVertexArray->SetIndexBuffer(cubeIB);
		delete[] cubeIndices;

		s_Data3D.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data3D.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[s_Data3D.MaxTextureSlots];
		for (int i = 0; i < s_Data3D.MaxTextureSlots; i++)
			samplers[i] = i;

		// Reusing the 2D texture shader for now, ideally 3D needs its own shader with lighting
		s_Data3D.TextureShader = Shader::Create("assets/shaders/Texture2DShader.glsl");
		s_Data3D.TextureShader->Bind();
		s_Data3D.TextureShader->SetIntArray("u_Textures", samplers, s_Data3D.MaxTextureSlots);

		s_Data3D.ModelShader = Shader::Create("assets/shaders/ModelShader.glsl");

		s_Data3D.TextureSlots[0] = s_Data3D.WhiteTexture;

		// Define the 24 vertices for a unit cube centered at origin
		// Front
		s_Data3D.CubeVertexPositions[0]  = { -0.5f, -0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[1]  = {  0.5f, -0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[2]  = {  0.5f,  0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[3]  = { -0.5f,  0.5f,  0.5f, 1.0f };
		// Back
		s_Data3D.CubeVertexPositions[4]  = {  0.5f, -0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[5]  = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[6]  = { -0.5f,  0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[7]  = {  0.5f,  0.5f, -0.5f, 1.0f };
		// Left
		s_Data3D.CubeVertexPositions[8]  = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[9]  = { -0.5f, -0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[10] = { -0.5f,  0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[11] = { -0.5f,  0.5f, -0.5f, 1.0f };
		// Right
		s_Data3D.CubeVertexPositions[12] = {  0.5f, -0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[13] = {  0.5f, -0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[14] = {  0.5f,  0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[15] = {  0.5f,  0.5f,  0.5f, 1.0f };
		// Top
		s_Data3D.CubeVertexPositions[16] = { -0.5f,  0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[17] = {  0.5f,  0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[18] = {  0.5f,  0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[19] = { -0.5f,  0.5f, -0.5f, 1.0f };
		// Bottom
		s_Data3D.CubeVertexPositions[20] = { -0.5f, -0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[21] = {  0.5f, -0.5f, -0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[22] = {  0.5f, -0.5f,  0.5f, 1.0f };
		s_Data3D.CubeVertexPositions[23] = { -0.5f, -0.5f,  0.5f, 1.0f };

		for (int i = 0; i < 6; i++)
		{
			s_Data3D.CubeTexCoords[i * 4 + 0] = { 0.0f, 0.0f };
			s_Data3D.CubeTexCoords[i * 4 + 1] = { 1.0f, 0.0f };
			s_Data3D.CubeTexCoords[i * 4 + 2] = { 1.0f, 1.0f };
			s_Data3D.CubeTexCoords[i * 4 + 3] = { 0.0f, 1.0f };
		}
	}

	void Renderer3D::Shutdown()
	{
		QL_PROFILE_FUNCTION();
		delete[] s_Data3D.CubeVertexBufferBase;
	}

	void Renderer3D::BeginScene(OrthographicCamera& camera)
	{
		QL_PROFILE_FUNCTION();

		s_Data3D.TextureShader->Bind();
		s_Data3D.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data3D.ModelShader->Bind();
		s_Data3D.ModelShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data3D.CubeIndexCount = 0;
		s_Data3D.CubeVertexBufferPtr = s_Data3D.CubeVertexBufferBase;

		s_Data3D.TextureSlotIndex = 1;
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera)
	{
		QL_PROFILE_FUNCTION();

		s_Data3D.TextureShader->Bind();
		s_Data3D.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data3D.ModelShader->Bind();
		s_Data3D.ModelShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		s_Data3D.CubeIndexCount = 0;
		s_Data3D.CubeVertexBufferPtr = s_Data3D.CubeVertexBufferBase;

		s_Data3D.TextureSlotIndex = 1;
	}

	void Renderer3D::DrawModel(const glm::mat4& transform, const Model& model, const glm::vec4& color, int entityID)
	{
		QL_PROFILE_FUNCTION();

		s_Data3D.ModelShader->Bind();
		s_Data3D.ModelShader->SetMat4("u_Transform", transform);
		s_Data3D.ModelShader->SetFloat4("u_Color", color);
		s_Data3D.ModelShader->SetInt("u_EntityID", entityID);

		model.Draw();
	}

	void Renderer3D::EndScene()
	{
		QL_PROFILE_FUNCTION();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data3D.CubeVertexBufferPtr - (uint8_t*)s_Data3D.CubeVertexBufferBase);
		s_Data3D.CubeVertexBuffer->SetData(s_Data3D.CubeVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer3D::Flush()
	{
		if (s_Data3D.CubeIndexCount == 0)
			return;

		s_Data3D.TextureShader->Bind();

		for (uint32_t i = 0; i < s_Data3D.TextureSlotIndex; i++)
			s_Data3D.TextureSlots[i]->Bind(i);

		RenderCommand::DrawIndexed(s_Data3D.CubeVertexArray, s_Data3D.CubeIndexCount);
		s_Data3D.Stats.DrawCalls++;
	}

	void Renderer3D::FlushAndReset()
	{
		EndScene();

		s_Data3D.CubeIndexCount = 0;
		s_Data3D.CubeVertexBufferPtr = s_Data3D.CubeVertexBufferBase;
		s_Data3D.TextureSlotIndex = 1;
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color, int entityID)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), size);
		DrawCube(transform, color, entityID);
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		QL_PROFILE_FUNCTION();

		if (s_Data3D.CubeIndexCount >= s_Data3D.MaxIndices)
			FlushAndReset();

		const float textureIndex = 0.0f;
		const float tilingFactor = 1.0f;

		for (uint32_t i = 0; i < 24; i++)
		{
			s_Data3D.CubeVertexBufferPtr->Position = transform * s_Data3D.CubeVertexPositions[i];
			s_Data3D.CubeVertexBufferPtr->Color = color;
			s_Data3D.CubeVertexBufferPtr->TexCoord = s_Data3D.CubeTexCoords[i];
			s_Data3D.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data3D.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data3D.CubeVertexBufferPtr->EntityID = entityID;
			s_Data3D.CubeVertexBufferPtr++;
		}

		s_Data3D.CubeIndexCount += 36;
		s_Data3D.Stats.CubeCount++;
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, const float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		QL_PROFILE_FUNCTION();

		if (s_Data3D.CubeIndexCount >= s_Data3D.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data3D.TextureSlotIndex; i++)
		{
			if (*s_Data3D.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data3D.TextureSlotIndex >= s_Data3D.MaxTextureSlots)
				FlushAndReset();

			textureIndex = (float)s_Data3D.TextureSlotIndex;
			s_Data3D.TextureSlots[s_Data3D.TextureSlotIndex] = texture;
			s_Data3D.TextureSlotIndex++;
		}

		for (uint32_t i = 0; i < 24; i++)
		{
			s_Data3D.CubeVertexBufferPtr->Position = transform * s_Data3D.CubeVertexPositions[i];
			s_Data3D.CubeVertexBufferPtr->Color = tintColor;
			s_Data3D.CubeVertexBufferPtr->TexCoord = s_Data3D.CubeTexCoords[i];
			s_Data3D.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data3D.CubeVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data3D.CubeVertexBufferPtr->EntityID = entityID;
			s_Data3D.CubeVertexBufferPtr++;
		}

		s_Data3D.CubeIndexCount += 36;
		s_Data3D.Stats.CubeCount++;
	}

	void Renderer3D::ResetStats()
	{
		memset(&s_Data3D.Stats, 0, sizeof(Statistics));
	}

	Renderer3D::Statistics Renderer3D::GetStatistics()
	{
		return s_Data3D.Stats;
	}
}
