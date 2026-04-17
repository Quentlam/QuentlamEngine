#pragma once

#include <algorithm>
#include <cmath>

namespace Quentlam::EditorToolbarLayout
{
	inline constexpr float kMinButtonSize = 24.0f;
	inline constexpr float kMaxButtonSize = 34.0f;
	inline constexpr float kBaseButtonSize = 28.0f;
	inline constexpr float kMinButtonSpacing = 4.0f;
	inline constexpr float kBaseButtonSpacing = 6.0f;
	inline constexpr float kOuterPadding = 8.0f;
	inline constexpr float kGroupPaddingX = 6.0f;
	inline constexpr float kGroupPaddingY = 4.0f;
	inline constexpr float kSeparatorGap = 10.0f;
	inline constexpr float kToolbarHeight = 44.0f;

	struct Metrics
	{
		float DpiScale = 1.0f;
		float ToolbarHeight = kToolbarHeight;
		float OuterPadding = kOuterPadding;
		float GroupPaddingX = kGroupPaddingX;
		float GroupPaddingY = kGroupPaddingY;
		float ButtonSize = kBaseButtonSize;
		float ButtonSpacing = kBaseButtonSpacing;
		float GroupWidth = kBaseButtonSize;
		float GroupStartX = kOuterPadding;
		float GroupPanelStartX = kOuterPadding;
		float GroupPanelWidth = kBaseButtonSize + (kGroupPaddingX * 2.0f);
		float AddButtonX = kOuterPadding;
		float AddPanelStartX = kOuterPadding;
		float AddPanelWidth = kBaseButtonSize + (kGroupPaddingX * 2.0f);
		float PanelY = kOuterPadding;
		float PanelHeight = kToolbarHeight - (kOuterPadding * 2.0f);
		float CornerRounding = 6.0f;
		float IconInset = 6.0f;
		float SeparatorGap = kSeparatorGap;
		bool UseCompactLayout = false;
		bool HasOverlap = false;
		bool IsClipped = false;
	};

	inline Metrics Calculate(float contentWidth, float windowHeight, int buttonCount, float uiScale = 1.0f, float dpiScale = 1.0f)
	{
		Metrics metrics;
		const float scale = std::max(uiScale, 1.0f) * std::max(dpiScale, 1.0f);
		const int clampedButtonCount = std::max(buttonCount, 1);

		metrics.DpiScale = std::max(dpiScale, 1.0f);
		metrics.ToolbarHeight = std::max(kToolbarHeight * scale, windowHeight);
		metrics.OuterPadding = std::round(kOuterPadding * scale);
		metrics.GroupPaddingX = std::round(kGroupPaddingX * scale);
		metrics.GroupPaddingY = std::round(kGroupPaddingY * scale);
		metrics.SeparatorGap = std::round(kSeparatorGap * scale);
		metrics.CornerRounding = std::round(6.0f * scale);
		metrics.IconInset = std::round(6.0f * scale);
		metrics.PanelY = metrics.OuterPadding;
		metrics.PanelHeight = std::max(24.0f * scale, windowHeight - (metrics.OuterPadding * 2.0f));

		const float maxButtonFromHeight = std::max(kMinButtonSize * scale, metrics.PanelHeight - (metrics.GroupPaddingY * 2.0f));
		metrics.ButtonSize = std::clamp(std::round(maxButtonFromHeight), kMinButtonSize * scale, kMaxButtonSize * scale);
		metrics.ButtonSpacing = std::max(std::round(kBaseButtonSpacing * scale), kMinButtonSpacing * scale);
		metrics.GroupWidth = metrics.ButtonSize * clampedButtonCount + metrics.ButtonSpacing * std::max(0, clampedButtonCount - 1);
		metrics.GroupPanelWidth = metrics.GroupWidth + (metrics.GroupPaddingX * 2.0f);
		metrics.AddPanelWidth = metrics.ButtonSize + (metrics.GroupPaddingX * 2.0f);
		metrics.AddButtonX = std::max(metrics.OuterPadding + metrics.GroupPaddingX, contentWidth - metrics.OuterPadding - metrics.GroupPaddingX - metrics.ButtonSize);
		metrics.AddPanelStartX = std::max(metrics.OuterPadding, metrics.AddButtonX - metrics.GroupPaddingX);
		metrics.GroupPanelStartX = std::max(metrics.OuterPadding, (contentWidth - metrics.GroupPanelWidth) * 0.5f);
		metrics.GroupStartX = metrics.GroupPanelStartX + metrics.GroupPaddingX;

		const float requiredGap = metrics.SeparatorGap;
		const float groupPanelRight = metrics.GroupPanelStartX + metrics.GroupPanelWidth;
		const float addPanelLeft = metrics.AddPanelStartX;
		metrics.HasOverlap = groupPanelRight + requiredGap > addPanelLeft;

		if (metrics.HasOverlap)
		{
			metrics.UseCompactLayout = true;
			metrics.ButtonSpacing = std::max(kMinButtonSpacing * scale, metrics.ButtonSpacing - std::round(2.0f * scale));
			metrics.ButtonSize = std::max(kMinButtonSize * scale, metrics.ButtonSize - std::round(2.0f * scale));
			metrics.GroupWidth = metrics.ButtonSize * clampedButtonCount + metrics.ButtonSpacing * std::max(0, clampedButtonCount - 1);
			metrics.GroupPanelWidth = metrics.GroupWidth + (metrics.GroupPaddingX * 2.0f);
			metrics.AddPanelWidth = metrics.ButtonSize + (metrics.GroupPaddingX * 2.0f);
			metrics.AddButtonX = std::max(metrics.OuterPadding + metrics.GroupPaddingX, contentWidth - metrics.OuterPadding - metrics.GroupPaddingX - metrics.ButtonSize);
			metrics.AddPanelStartX = std::max(metrics.OuterPadding, metrics.AddButtonX - metrics.GroupPaddingX);
			metrics.GroupPanelStartX = std::max(metrics.OuterPadding, metrics.AddPanelStartX - requiredGap - metrics.GroupPanelWidth);
			metrics.GroupStartX = metrics.GroupPanelStartX + metrics.GroupPaddingX;
		}

		const float contentRight = std::max(contentWidth, 0.0f);
		metrics.IsClipped = metrics.GroupPanelStartX < 0.0f
			|| (metrics.GroupPanelStartX + metrics.GroupPanelWidth) > contentRight
			|| metrics.AddPanelStartX < 0.0f
			|| (metrics.AddPanelStartX + metrics.AddPanelWidth) > contentRight;

		return metrics;
	}
}
