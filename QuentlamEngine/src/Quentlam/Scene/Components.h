#pragma once

#include <glm/glm.hpp>
#include "Quentlam/Renderer/Camera.h"


namespace Quentlam
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string tag)
			:Tag(tag) {
		}
	};


	struct TransformComponent
	{
		glm::mat4 Transform{ 1.0f };
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform)
			:Transform(transform) {
		}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () { return Transform; }

	};

	struct SpriteTransformComponent
	{
		glm::vec4 Color{ 1.0f,1.0f,1.0f,1.0f };

		SpriteTransformComponent() = default;
		SpriteTransformComponent(const SpriteTransformComponent&) = default;
		SpriteTransformComponent(const glm::vec4& color)
			:Color(color) {
		}
	};

	struct CubeRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

		CubeRendererComponent() = default;
		CubeRendererComponent(const CubeRendererComponent&) = default;
		CubeRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct PrimitiveRendererComponent
	{
		enum class PrimitiveType { Cube, Sphere, Cylinder, Capsule, Cone, Torus };
		PrimitiveType Type = PrimitiveType::Cube;
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		int Segments = 16;
		float Radius = 0.5f;
		float Height = 1.0f;

		PrimitiveRendererComponent() = default;
		PrimitiveRendererComponent(const PrimitiveRendererComponent&) = default;
		PrimitiveRendererComponent(PrimitiveType type) : Type(type) {}
	};

	struct CameraComponent
	{
		Quentlam::Camera Camera;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const glm::mat4& projection)
			:Camera(projection) {
		}
	};

	// =========================================================================
	// 2D Physics Components
	// =========================================================================

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime body
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		// TODO: Material properties
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime fixture
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	// =========================================================================
	// 3D Physics Components
	// =========================================================================

	struct Rigidbody3DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;

		float Mass = 1.0f;
		bool FixedRotation = false;

		// Storage for runtime body (Jolt Physics Body ID)
		uint32_t RuntimeBodyID = 0xFFFFFFFF; // JPH::BodyID::cInvalidBodyID

		Rigidbody3DComponent() = default;
		Rigidbody3DComponent(const Rigidbody3DComponent&) = default;
	};

	struct BoxCollider3DComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };
		glm::vec3 HalfExtent = { 0.5f, 0.5f, 0.5f };

		float Friction = 0.5f;
		float Restitution = 0.0f;

		BoxCollider3DComponent() = default;
		BoxCollider3DComponent(const BoxCollider3DComponent&) = default;
	};

}