#pragma once

#include <algorithm>
#include <cmath>

namespace Quentlam::EditorToolbarLayout
{
	inline constexpr float kMinButtonSize = 34.0f;
	inline constexpr float kMaxButtonSize = 52.0f;
	inline constexpr float kBaseButtonSize = 42.0f;
	inline constexpr float kHorizontalPadding = 12.0f;
	inline constexpr float kVerticalPadding = 8.0f;
	inline constexpr float kToolbarTopMargin = 12.0f;

	struct Metrics
	{
		float DpiScale = 1.0f;
		float ButtonSize = kBaseButtonSize;
		float WindowWidth = kBaseButtonSize + (kHorizontalPadding * 2.0f);
		float WindowHeight = kBaseButtonSize + (kVerticalPadding * 2.0f);
		float WindowX = 0.0f;
		float WindowY = kToolbarTopMargin;
		float ButtonX = kHorizontalPadding;
		float ButtonY = kVerticalPadding;
		float CornerRounding = 8.0f;
		float IconInset = 8.0f;
		bool IsClipped = false;
		bool IsClamped = false;
	};

	inline Metrics Calculate(float viewportWidth, float viewportTop, float uiScale = 1.0f, float dpiScale = 1.0f)
	{
		Metrics metrics;
		const float scale = std::max(uiScale, 1.0f) * std::max(dpiScale, 1.0f);
		const float availableWidth = std::max(viewportWidth, 0.0f);

		metrics.DpiScale = std::max(dpiScale, 1.0f);
		metrics.ButtonSize = std::clamp(std::round(kBaseButtonSize * scale), kMinButtonSize * scale, kMaxButtonSize * scale);
		const float paddingX = std::round(kHorizontalPadding * scale);
		const float paddingY = std::round(kVerticalPadding * scale);
		metrics.WindowWidth = metrics.ButtonSize + (paddingX * 2.0f);
		metrics.WindowHeight = metrics.ButtonSize + (paddingY * 2.0f);
		metrics.ButtonX = paddingX;
		metrics.ButtonY = paddingY;
		metrics.WindowY = viewportTop + std::round(kToolbarTopMargin * scale);
		metrics.CornerRounding = std::round(10.0f * scale);
		metrics.IconInset = std::round(8.0f * scale);

		float centeredX = std::round((availableWidth - metrics.WindowWidth) * 0.5f);
		if (centeredX < 0.0f)
		{
			metrics.IsClamped = true;
			centeredX = 0.0f;
		}

		if (centeredX + metrics.WindowWidth > availableWidth)
		{
			metrics.IsClamped = true;
			centeredX = std::max(0.0f, availableWidth - metrics.WindowWidth);
		}

		metrics.WindowX = centeredX;
		metrics.IsClipped = metrics.WindowWidth > availableWidth;

		return metrics;
	}
}
