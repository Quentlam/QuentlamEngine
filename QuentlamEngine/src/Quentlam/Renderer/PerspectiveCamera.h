#pragma once

#include "qlpch.h"
#include <glm/glm.hpp>
#include "Quentlam/Core/Base.h"

namespace Quentlam
{
	class QUENTLAM_API PerspectiveCamera
	{
	public:
		PerspectiveCamera(float fov, float aspectRatio, float nearClip = 0.1f, float farClip = 1000.0f);
		
		void SetProjection(float fov, float aspectRatio, float nearClip = 0.1f, float farClip = 1000.0f);

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		// Rotation in degrees (Pitch, Yaw, Roll)
		const glm::vec3& GetRotation() const { return m_Rotation; }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f }; // x = pitch, y = yaw, z = roll
	};
}
