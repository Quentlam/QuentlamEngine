#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ScenePresetTypes.h"
#include "ScenePresetUserSettings.generated.h"

UCLASS(Config=ScenePresets, defaultconfig, meta=(DisplayName="Scene Preset Settings"))
class SCENEPRESETMANAGER_API UScenePresetUserSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UScenePresetUserSettings();

	// 上次保存的场景预设数据
	UPROPERTY(Config, EditAnywhere, Category = "Presets")
	FScenePreset LastPreset;

	// 是否有未保存的修改
	UPROPERTY(Transient)
	bool bDirty;
};