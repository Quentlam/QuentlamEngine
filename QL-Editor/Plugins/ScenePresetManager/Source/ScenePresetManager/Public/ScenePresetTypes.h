#pragma once

#include "CoreMinimal.h"
#include "ScenePresetTypes.generated.h"

USTRUCT(BlueprintType)
struct FScenePreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Preset")
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Preset")
	TArray<FString> LevelList;

	// Simplification for Actor transforms/properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Preset")
	TMap<FString, FTransform> ActorTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Preset")
	FVector CameraPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene Preset")
	FRotator CameraRotation = FRotator::ZeroRotator;
};