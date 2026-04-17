#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IScenePresetManager.generated.h"

UINTERFACE(MinimalAPI)
class UScenePresetManager : public UInterface
{
	GENERATED_BODY()
};

class SCENEPRESETMANAGER_API IScenePresetManager
{
	GENERATED_BODY()

public:
	// 异步保存当前场景预设
	virtual void SaveCurrentScenePresetAsync() = 0;

	// 恢复上次的场景预设
	virtual void RestoreLastScenePreset() = 0;

	// 拦截退出并提示保存
	virtual bool HandleEditorExit() = 0;
};