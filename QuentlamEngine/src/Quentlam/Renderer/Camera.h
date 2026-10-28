#pragma once

#include <glm/glm.hpp>


namespace Quentlam {

	class Camera {
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			:m_Projection(projection) {
		}
		const glm::mat4& GetProjection() const { return m_Projection; }
		void SetProjection(const glm::mat4& projection) { m_Projection = projection; }

	private:
		glm::mat4 m_Projection{ 1.0f };

	};

}	