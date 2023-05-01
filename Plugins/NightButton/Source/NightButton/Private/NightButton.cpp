// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightButton.h"
#include "NightButtonStyle.h"
#include "NightButtonCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Engine/PostProcessVolume.h"

static const FName NightButtonTabName("NightButton");

#define LOCTEXT_NAMESPACE "FNightButtonModule"

void FNightButtonModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FNightButtonStyle::Initialize();
	FNightButtonStyle::ReloadTextures();

	FNightButtonCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FNightButtonCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FNightButtonModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FNightButtonModule::RegisterMenus));
}

void FNightButtonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FNightButtonStyle::Shutdown();

	FNightButtonCommands::Unregister();
}

void FNightButtonModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::FromString("Changing scene to Night");
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	AActor* FoundActor;
	FoundActor = FindActor(ADirectionalLight::StaticClass());

	if (FoundActor)
	{
		ADirectionalLight* Sun = Cast<ADirectionalLight>(FoundActor);
		Sun->GetLightComponent()->SetIntensity(1.f);
	}

	FoundActor = FindActor(APostProcessVolume::StaticClass());

	APostProcessVolume* PPVol;

	if (!FoundActor)
	{
		DialogText = FText::FromString("PostProcessVolume Not Found, Creating One");
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		FoundActor = AddActor(APostProcessVolume::StaticClass());
	}

	PPVol = Cast<APostProcessVolume>(FoundActor);

	PPVol->Settings.AutoExposureBias = -3.f;
	PPVol->Settings.bOverride_AutoExposureBias = true;
	PPVol->bUnbound = true;
}

void FNightButtonModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FNightButtonCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FNightButtonCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

AActor* FNightButtonModule::FindActor(TSubclassOf<AActor> ActorClass)
{
	TArray<AActor*> FoundActors;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World)
	{
		UGameplayStatics::GetAllActorsOfClass(World, ActorClass, FoundActors);
		if (FoundActors.Num() > 0)
		{
			return FoundActors[0];
		}
	}
	return nullptr;
}

AActor* FNightButtonModule::AddActor(TSubclassOf<AActor> ActorClass)
{
	ULevel* Level = GEditor->GetEditorWorldContext().World()->GetCurrentLevel();
	return GEditor->AddActor(Level, ActorClass, FTransform());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNightButtonModule, NightButton)