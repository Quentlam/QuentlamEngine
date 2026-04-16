#pragma once

#include <glm/glm.hpp>



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
}