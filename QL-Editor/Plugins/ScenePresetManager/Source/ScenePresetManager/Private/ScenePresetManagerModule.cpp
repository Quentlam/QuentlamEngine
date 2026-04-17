#include "ScenePresetManagerModule.h"

#define LOCTEXT_NAMESPACE "FScenePresetManagerModule"

void FScenePresetManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
}

void FScenePresetManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScenePresetManagerModule, ScenePresetManager)