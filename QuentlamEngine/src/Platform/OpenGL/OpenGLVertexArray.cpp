#include "qlpch.h"
#include "OpenGLVertexArray.h"
#include "glad/glad.h"


namespace Quentlam
{

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Quentlam::ShaderDataType::Float:		return GL_FLOAT;
		case Quentlam::ShaderDataType::Float2:		return GL_FLOAT;
		case Quentlam::ShaderDataType::Float3:		return GL_FLOAT;
		case Quentlam::ShaderDataType::Float4:		return GL_FLOAT;
		case Quentlam::ShaderDataType::Mat3:		return GL_FLOAT;
		case Quentlam::ShaderDataType::Mat4:		return GL_FLOAT;
		case Quentlam::ShaderDataType::Int:			return GL_INT;
		case Quentlam::ShaderDataType::Int2:		return GL_INT;
		case Quentlam::ShaderDataType::Int3:		return GL_INT;
		case Quentlam::ShaderDataType::Int4:		return GL_INT;
		case Quentlam::ShaderDataType::Bool:		return GL_BOOL;
		}
		QL_CORE_ASSERTS(false, "Unknodw ShaderDataType!");
		return 0;
	}


	OpenGLVertexArray::OpenGLVertexArray()
	{
		QL_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		QL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		QL_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);
	//glBindBuffer(GL_ARRAY_BUFFER, posVbo);//只有绑定了vbo，下面的描述才会跟这个vbo相关
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		QL_PROFILE_FUNCTION();

		QL_CORE_ASSERTS(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");
	
		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);

	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		QL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}
	OpenGLVertexArray::~OpenGLVertexArray()
	{
		QL_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}
}