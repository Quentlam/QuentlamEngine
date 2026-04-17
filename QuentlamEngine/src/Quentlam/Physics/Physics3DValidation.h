#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace Quentlam::Physics3DValidation
{
	inline constexpr float kMinHalfExtent = 0.01f;
	inline constexpr float kMaxConvexRadius = 0.05f;
	inline constexpr float kMinDynamicMass = 0.001f;

	inline bool IsFinite(float value)
	{
		return std::isfinite(value) != 0;
	}

	inline bool IsFiniteVec3(const glm::vec3& value)
	{
		return IsFinite(value.x) && IsFinite(value.y) && IsFinite(value.z);
	}

	inline glm::vec3 SanitizeHalfExtent(const glm::vec3& halfExtent, const glm::vec3& scale)
	{
		glm::vec3 sanitized = glm::abs(halfExtent * scale);
		if (!IsFiniteVec3(sanitized))
			return glm::vec3(kMinHalfExtent);

		sanitized.x = std::max(sanitized.x, kMinHalfExtent);
		sanitized.y = std::max(sanitized.y, kMinHalfExtent);
		sanitized.z = std::max(sanitized.z, kMinHalfExtent);
		return sanitized;
	}

	inline float CalculateConvexRadius(const glm::vec3& extents)
	{
		if (!IsFiniteVec3(extents))
			return 0.0f;

		float minExtent = std::min(extents.x, std::min(extents.y, extents.z));
		if (minExtent <= 0.0f)
			return 0.0f;

		return std::clamp(minExtent * 0.49f, 0.0f, kMaxConvexRadius);
	}

	inline float SanitizeMass(float mass)
	{
		if (!IsFinite(mass))
			return kMinDynamicMass;

		return std::max(mass, kMinDynamicMass);
	}
}
