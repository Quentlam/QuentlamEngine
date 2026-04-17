#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "IScenePresetManager.h"
#include "ScenePresetManagerSubsystem.generated.h"

UCLASS()
class SCENEPRESETMANAGER_API UScenePresetManagerSubsystem : public UEditorSubsystem, public IScenePresetManager
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// IScenePresetManager 接口
	virtual void SaveCurrentScenePresetAsync() override;
	virtual void RestoreLastScenePreset() override;
	virtual bool HandleEditorExit() override;

private:
	void OnPreSaveWorld(uint32 SaveFlags, UWorld* World);
	void OnEditorStartup();
	bool OnPreExit();

	void RegisterCommands();
	void UnregisterCommands();

	void QuickSaveAction();

	TSharedPtr<class FUICommandList> PluginCommands;
};