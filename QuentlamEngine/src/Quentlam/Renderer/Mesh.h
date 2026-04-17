#pragma once
#include "Quentlam/Renderer/Buffer.h"
#include "Quentlam/Renderer/VertexArray.h"
#include "Quentlam/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <vector>

namespace Quentlam {

	struct QUENTLAM_API Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	class QUENTLAM_API Mesh {
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void InitGPU();
		void Draw() const;

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Ref<VertexArray> m_VertexArray;
	};
}
