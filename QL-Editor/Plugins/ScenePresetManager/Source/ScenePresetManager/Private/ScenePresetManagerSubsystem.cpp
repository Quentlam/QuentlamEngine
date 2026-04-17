#include "ScenePresetManagerSubsystem.h"
#include "ScenePresetUserSettings.h"
#include "Editor.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "Async/AsyncWork.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Framework/Application/SlateApplication.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FScenePresetManagerModule"

class FScenePresetCommands : public TCommands<FScenePresetCommands>
{
public:
	FScenePresetCommands()
		: TCommands<FScenePresetCommands>(TEXT("ScenePresetManager"), NSLOCTEXT("Contexts", "ScenePresetManager", "Scene Preset Manager Plugin"), NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(QuickSave, "Quick Save Scene Preset", "Quickly save the current scene preset", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::S));
	}

	TSharedPtr<FUICommandInfo> QuickSave;
};

class FAsyncSavePresetTask : public FNonAbandonableTask
{
	FScenePreset PresetData;
public:
	FAsyncSavePresetTask(const FScenePreset& InPresetData) : PresetData(InPresetData) {}

	void DoWork()
	{
		// Simulate saving delay and write to config
		UScenePresetUserSettings* Settings = GetMutableDefault<UScenePresetUserSettings>();
		Settings->LastPreset = PresetData;
		Settings->bDirty = false;
		Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());
		GConfig->Flush(true, GEngineIni);
	}
	
	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
};

void UScenePresetManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FEditorDelegates::PreSaveWorld.AddUObject(this, &UScenePresetManagerSubsystem::OnPreSaveWorld);
	FEditorDelegates::OnEditorInitialized.AddUObject(this, &UScenePresetManagerSubsystem::OnEditorStartup);
	FEditorDelegates::PreExit.AddUObject(this, &UScenePresetManagerSubsystem::OnPreExit); // UE PreExit delegate
	
	RegisterCommands();
}

void UScenePresetManagerSubsystem::Deinitialize()
{
	UnregisterCommands();

	FEditorDelegates::PreSaveWorld.RemoveAll(this);
	FEditorDelegates::OnEditorInitialized.RemoveAll(this);
	FEditorDelegates::PreExit.RemoveAll(this);

	Super::Deinitialize();
}

void UScenePresetManagerSubsystem::RegisterCommands()
{
	FScenePresetCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FScenePresetCommands::Get().QuickSave,
		FExecuteAction::CreateUObject(this, &UScenePresetManagerSubsystem::QuickSaveAction),
		FCanExecuteAction());

	// Hook into LevelEditor
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());
}

void UScenePresetManagerSubsystem::UnregisterCommands()
{
	FScenePresetCommands::Unregister();
}

void UScenePresetManagerSubsystem::QuickSaveAction()
{
	SaveCurrentScenePresetAsync();
}

void UScenePresetManagerSubsystem::SaveCurrentScenePresetAsync()
{
	if (!GEditor || !GEditor->GetEditorWorldContext().World())
		return;

	UWorld* World = GEditor->GetEditorWorldContext().World();
	
	FScenePreset Preset;
	Preset.MapName = World->GetMapName();
	
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		Preset.ActorTransforms.Add(Actor->GetActorNameOrLabel(), Actor->GetActorTransform());
	}
	
	(new FAutoDeleteAsyncTask<FAsyncSavePresetTask>(Preset))->StartBackgroundTask();
}

void UScenePresetManagerSubsystem::RestoreLastScenePreset()
{
	UScenePresetUserSettings* Settings = GetMutableDefault<UScenePresetUserSettings>();
	if (Settings->LastPreset.MapName.IsEmpty())
		return;

	// In real implementation: Open the Level if missing show message
	// For demonstration, we simulate opening and restoring
	UE_LOG(LogTemp, Log, TEXT("Restored scene preset: %s"), *Settings->LastPreset.MapName);
}

void UScenePresetManagerSubsystem::OnPreSaveWorld(uint32 SaveFlags, UWorld* World)
{
	SaveCurrentScenePresetAsync();
}

void UScenePresetManagerSubsystem::OnEditorStartup()
{
	RestoreLastScenePreset();
}

bool UScenePresetManagerSubsystem::OnPreExit()
{
	return HandleEditorExit();
}

bool UScenePresetManagerSubsystem::HandleEditorExit()
{
	UScenePresetUserSettings* Settings = GetMutableDefault<UScenePresetUserSettings>();
	if (Settings->bDirty)
	{
		EAppReturnType::Type Result = FMessageDialog::Open(
			EAppMsgType::YesNoCancel,
			LOCTEXT("UnsavedPreset", "场景已修改，是否保存预设？")
		);

		if (Result == EAppReturnType::Yes)
		{
			SaveCurrentScenePresetAsync();
			return true;
		}
		else if (Result == EAppReturnType::Cancel)
		{
			// Block exit? UE PreExit doesn't allow blocking easily, but conceptually:
			// In Slate, we would handle Window close event to block.
			return false;
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE