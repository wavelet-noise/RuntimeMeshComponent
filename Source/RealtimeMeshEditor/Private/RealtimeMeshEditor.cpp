// Copyright (c) 2015-2025 TriAxis Games, L.L.C. All Rights Reserved.

#include "RealtimeMeshEditor.h"

#include "Editor.h"
#include "ToolMenus.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshMenuExtension.h"

#define LOCTEXT_NAMESPACE "RealtimeMeshEditorModule"

static bool GRealtimeMeshNotifyLumenUseInCore = false;
static FAutoConsoleVariableRef CVarRealtimeMeshNotifyLumenUseInCore(
	TEXT("RealtimeMesh.EnableNotificationsForLumenSupportInPro"),
	GRealtimeMeshNotifyLumenUseInCore,
	TEXT("Should we notify the user when they have an RMC in a scene with Lumen active but don't have RMC-Pro."),
	ECVF_Default);
	
void FRealtimeMeshEditorModule::StartupModule()
{
#if RMC_ENGINE_ABOVE_5_4
	LoadSettings();
#endif
	FRealtimeMeshEditorStyle::Initialize();
	FRealtimeMeshEditorStyle::ReloadTextures();
	FRealtimeMeshEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FRealtimeMeshEditorCommands::Get().MarketplaceProAction,
		FExecuteAction::CreateRaw(this, &FRealtimeMeshEditorModule::MarketplaceProButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRealtimeMeshEditorCommands::Get().MarketplaceCoreAction,
		FExecuteAction::CreateRaw(this, &FRealtimeMeshEditorModule::MarketplaceCoreButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRealtimeMeshEditorCommands::Get().DiscordAction,
		FExecuteAction::CreateRaw(this, &FRealtimeMeshEditorModule::DiscordButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRealtimeMeshEditorCommands::Get().DocumentationAction,
		FExecuteAction::CreateRaw(this, &FRealtimeMeshEditorModule::DocumentationButtonClicked),
		FCanExecuteAction());
	PluginCommands->MapAction(
		FRealtimeMeshEditorCommands::Get().IssuesAction,
		FExecuteAction::CreateRaw(this, &FRealtimeMeshEditorModule::IssuesButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRealtimeMeshEditorModule::RegisterMenus));
}

void FRealtimeMeshEditorModule::ShutdownModule()
{	
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FRealtimeMeshEditorStyle::Shutdown();
	FRealtimeMeshEditorCommands::Unregister();
}


void FRealtimeMeshEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu");
		Menu = Menu->AddSubMenu("MainMenu", NAME_None, "RealtimeMesh", LOCTEXT("RealtimeMesh", "Realtime Mesh"), LOCTEXT("RealtimeMesh_Tooltip", "Open the Realtime Mesh menu"));

		FToolMenuSection& Section = Menu->FindOrAddSection("Tools");

		{
			Section.AddMenuEntryWithCommandList(FRealtimeMeshEditorCommands::Get().MarketplaceProAction, PluginCommands);
			Section.AddMenuEntryWithCommandList(FRealtimeMeshEditorCommands::Get().MarketplaceCoreAction, PluginCommands);
			Section.AddMenuEntryWithCommandList(FRealtimeMeshEditorCommands::Get().DiscordAction, PluginCommands);
			Section.AddMenuEntryWithCommandList(FRealtimeMeshEditorCommands::Get().DocumentationAction, PluginCommands);
			Section.AddMenuEntryWithCommandList(FRealtimeMeshEditorCommands::Get().IssuesAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("RealtimeMeshTools");
			{
				FToolMenuEntry MenuEntry = FToolMenuEntry::InitComboButton(
					"RealtimeMeshTools",
					FUIAction(),
					FOnGetContent::CreateStatic(&FRealtimeMeshEditorModule::GenerateToolbarMenuContent, PluginCommands),
					LOCTEXT("RealtimeMeshTools_Label", "Realtime Mesh"),
					LOCTEXT("RealtimeMeshTools_Tooltip", "Open the Realtime Mesh menu"),
					FSlateIcon(FRealtimeMeshEditorStyle::Get().GetStyleSetName(), "RealtimeMesh.MenuAction")
				);
				Section.AddEntry(MenuEntry);
			}
		}
	}
}

TSharedRef<SWidget> FRealtimeMeshEditorModule::GenerateToolbarMenuContent(TSharedPtr<FUICommandList> Commands)
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, Commands);

	MenuBuilder.BeginSection("RealtimeMesh", LOCTEXT("RealtimeMesh", "Realtime Mesh"));
	{
		MenuBuilder.AddMenuEntry(FRealtimeMeshEditorCommands::Get().MarketplaceProAction);
		MenuBuilder.AddMenuEntry(FRealtimeMeshEditorCommands::Get().MarketplaceCoreAction);
		MenuBuilder.AddMenuEntry(FRealtimeMeshEditorCommands::Get().DiscordAction);
		MenuBuilder.AddMenuEntry(FRealtimeMeshEditorCommands::Get().DocumentationAction);
		MenuBuilder.AddMenuEntry(FRealtimeMeshEditorCommands::Get().IssuesAction);
	}
	MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}

void FRealtimeMeshEditorModule::MarketplaceProButtonClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://www.unrealengine.com/marketplace/en-US/product/realtime-mesh-component-pro"), nullptr, nullptr);
}

void FRealtimeMeshEditorModule::MarketplaceCoreButtonClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://www.unrealengine.com/marketplace/en-US/product/runtime-mesh-component"), nullptr, nullptr);
}

void FRealtimeMeshEditorModule::DiscordButtonClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://discord.gg/KGvBBTv"), nullptr, nullptr);
}

void FRealtimeMeshEditorModule::DocumentationButtonClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://rmc.triaxis.games/"), nullptr, nullptr);
}

void FRealtimeMeshEditorModule::IssuesButtonClicked()
{
	FPlatformProcess::LaunchURL(TEXT("https://github.com/TriAxis-Games/RealtimeMeshComponent/issues"), nullptr, nullptr);
}

#if RMC_ENGINE_ABOVE_5_4
bool FRealtimeMeshEditorModule::IsProVersion()
{
	return true;
}

void FRealtimeMeshEditorModule::LoadSettings()
{
	const FString ConfigPath = FPaths::EngineUserDir() / TEXT("Saved") / TEXT("RealtimeMesh.ini");

	FConfigFile ConfigFile;
	ConfigFile.Read(ConfigPath);


	const auto ReadBool = [](const FConfigSection* Section, const TCHAR* Key)
	{
		if (const auto* T = Section->Find(Key))
		{
			return FCString::ToBool(*T->GetValue());
		}
		return false;
	};
	const auto ReadInt = [](const FConfigSection* Section, const TCHAR* Key)
	{
		if (const auto* T = Section->Find(Key))
		{
			if (FCString::IsNumeric(*T->GetValue()))
			{
				return FCString::Atoi64(*T->GetValue());
			}
		}
		return 0ll;
	};

	{
		const FConfigSection* NotificationSection = ConfigFile.FindOrAddConfigSection(TEXT("Notifications"));

		Settings.bShouldIgnoreLumenNotification = ReadBool(NotificationSection, TEXT("bShouldIgnoreLumenNotification"));
		Settings.bShouldIgnoreGeneralNotification = ReadBool(NotificationSection, TEXT("bShouldIgnoreGeneralNotification"));

		Settings.LastLumenNotificationTime = ReadInt(NotificationSection, TEXT("LastLumenNotificationTime"));
		Settings.LastGeneralNotificationTime = ReadInt(NotificationSection, TEXT("LastGeneralNotificationTime"));
	}
}

void FRealtimeMeshEditorModule::SaveSettings()
{
	const FString ConfigPath = FPaths::EngineUserDir() / TEXT("Saved") / TEXT("RealtimeMesh.ini");

	FConfigFile ConfigFile;

	{
		ConfigFile.AddToSection(TEXT("Notifications"), TEXT("bShouldIgnoreLumenNotification"), Settings.bShouldIgnoreLumenNotification ? TEXT("True") : TEXT("False"));
		ConfigFile.AddToSection(TEXT("Notifications"), TEXT("bShouldIgnoreGeneralNotification"), Settings.bShouldIgnoreGeneralNotification ? TEXT("True") : TEXT("False"));

		ConfigFile.AddToSection(TEXT("Notifications"), TEXT("LastLumenNotificationTime"), FString::FromInt(Settings.LastLumenNotificationTime));
		ConfigFile.AddToSection(TEXT("Notifications"), TEXT("LastGeneralNotificationTime"), FString::FromInt(Settings.LastGeneralNotificationTime));
	}

	ConfigFile.Write(ConfigPath);
}
#endif


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRealtimeMeshEditorModule, RealtimeMeshEditor)
