#include <cmath>
#include <iostream>

#include "EditorToolbarLayout.h"
#include "Quentlam/Physics/Physics3DValidation.h"

namespace
{
	int s_Failures = 0;

	void Expect(bool condition, const char* message)
	{
		if (!condition)
		{
			++s_Failures;
			std::cerr << "[FAIL] " << message << std::endl;
		}
	}

	void TestPhysics3DValidation()
	{
		using namespace Quentlam::Physics3DValidation;

		auto sanitizedThin = SanitizeHalfExtent({ 10.0f, 0.05f, 10.0f }, { 1.0f, 1.0f, 1.0f });
		Expect(sanitizedThin.y >= kMinHalfExtent, "Thin collider keeps a positive half extent.");

		float thinRadius = CalculateConvexRadius(sanitizedThin);
		Expect(thinRadius >= 0.0f && thinRadius < sanitizedThin.y, "Convex radius stays smaller than the thinnest axis.");

		auto sanitizedInvalid = SanitizeHalfExtent({ NAN, -1.0f, 0.0f }, { 1.0f, 2.0f, 3.0f });
		Expect(sanitizedInvalid.x == kMinHalfExtent, "Invalid extents fall back to minimum half extent.");
		Expect(sanitizedInvalid.y == kMinHalfExtent, "Negative extents are clamped to minimum half extent.");
		Expect(sanitizedInvalid.z == kMinHalfExtent, "Zero extents are clamped to minimum half extent.");

		Expect(SanitizeMass(0.0f) == kMinDynamicMass, "Zero mass is clamped to safe minimum.");
		Expect(SanitizeMass(-10.0f) == kMinDynamicMass, "Negative mass is clamped to safe minimum.");
		Expect(SanitizeMass(2.5f) == 2.5f, "Valid mass is preserved.");
	}

	void TestToolbarLayout()
	{
		using namespace Quentlam::EditorToolbarLayout;

		auto singleButton = Calculate(1200.0f, 52.0f, 1, 1.0f);
		Expect(singleButton.ButtonSize >= kMinButtonSize, "Toolbar button size respects minimum touch target.");
		Expect(!singleButton.HasOverlap, "Single-button toolbar does not overlap quick-add button.");
		Expect(!singleButton.IsClipped, "Single-button toolbar fits inside the toolbar.");
		Expect(singleButton.GroupPanelWidth > singleButton.GroupWidth, "Single-button toolbar reserves panel padding.");

		auto dualButton = Calculate(1200.0f, 52.0f, 2, 1.0f);
		Expect(dualButton.GroupWidth > singleButton.GroupWidth, "Two-button state reserves additional width.");
		Expect(!dualButton.HasOverlap, "Two-button toolbar keeps spacing from quick-add button.");
		Expect(!dualButton.IsClipped, "Two-button toolbar remains visible.");
		Expect(dualButton.AddPanelStartX > dualButton.GroupPanelStartX, "Quick-add panel stays to the right of the play group.");

		auto scaledLayout = Calculate(800.0f, 60.0f, 2, 1.5f, 1.25f);
		Expect(scaledLayout.ButtonSize >= kMinButtonSize * 1.25f, "Higher DPI enlarges toolbar buttons.");
		Expect(scaledLayout.ButtonSpacing >= kMinButtonSpacing * 1.25f, "Higher DPI enlarges toolbar spacing.");
		Expect(!scaledLayout.IsClipped, "Scaled toolbar remains visible.");

		auto compactLayout = Calculate(170.0f, 44.0f, 2, 1.0f, 1.0f);
		Expect(compactLayout.UseCompactLayout, "Narrow toolbars switch to compact spacing.");
		Expect(!compactLayout.IsClipped, "Compact toolbar avoids clipping in narrow widths.");
	}
}

int main()
{
	TestPhysics3DValidation();
	TestToolbarLayout();

	if (s_Failures > 0)
	{
		std::cerr << "[RESULT] " << s_Failures << " checks failed." << std::endl;
		return 1;
	}

	std::cout << "[RESULT] All regression checks passed." << std::endl;
	return 0;
}
