#include "qlpch.h"
#include "PerspectiveCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Quentlam
{
	PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
	{
		SetProjection(fov, aspectRatio, nearClip, farClip);
	}

	void PerspectiveCamera::SetProjection(float fov, float aspectRatio, float nearClip, float farClip)
	{
		QL_PROFILE_FUNCTION();
		// Ensure farClip is much greater than nearClip and > 0 to avoid invalid frustum
		float n = glm::max(nearClip, 0.01f);
		float f = glm::max(farClip, n + 1.0f);
		
		// Use right-handed coordinate system perspective projection with zero-to-one depth
		m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, n, f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		QL_PROFILE_FUNCTION();

		// Create rotation matrix from pitch, yaw, roll
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
							 glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
							 glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * rotation;

		// The view matrix is the inverse of the camera's transform matrix
		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}
