#include "qlpch.h"
#include "Mesh.h"
#include "Quentlam/Renderer/RenderCommand.h"

namespace Quentlam {

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_VertexArray = VertexArray::Create();

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create((float*)vertices.data(), (uint32_t)(vertices.size() * sizeof(Vertex)));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create((uint32_t*)indices.data(), (uint32_t)indices.size());
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}

	void Mesh::Draw() const
	{
		m_VertexArray->Bind();
		RenderCommand::DrawIndexed(m_VertexArray);
	}
}
