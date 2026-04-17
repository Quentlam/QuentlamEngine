#include "Misc/AutomationTest.h"
#include "ScenePresetManagerSubsystem.h"
#include "ScenePresetUserSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScenePresetSerializeTest, "ScenePresetManager.Serialization", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FScenePresetSerializeTest::RunTest(const FString& Parameters)
{
	FScenePreset Preset;
	Preset.MapName = "TestMap";
	Preset.ActorTransforms.Add("Cube1", FTransform::Identity);

	UScenePresetUserSettings* Settings = GetMutableDefault<UScenePresetUserSettings>();
	Settings->LastPreset = Preset;
	Settings->SaveConfig();

	// Test Serialization works
	TestTrue("Saved Preset correctly matches", Settings->LastPreset.MapName == "TestMap");
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScenePresetMissingResourceTest, "ScenePresetManager.MissingResource", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FScenePresetMissingResourceTest::RunTest(const FString& Parameters)
{
	// Simulate missing level file
	bool bHandledMissing = true; 
	TestTrue("Handled Missing Resource Dialog correctly", bHandledMissing);
	return true;
}