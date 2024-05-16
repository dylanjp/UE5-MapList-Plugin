// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapList.h"
#include "MapListStyle.h"
#include "MapListCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include <FileHelpers.h>
// Include necessary headers for content browsing and asset registry
#include "ContentBrowserModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include <Kismet/GameplayStatics.h>
#include "SLevelViewport.h"
#include "Engine/World.h"
#include "UnrealEdGlobals.h"
#include "Subsystems/AssetEditorSubsystem.h"


static const FName MapListTabName("MapList");
DECLARE_DELEGATE_OneParam(FOnMapSelected, const FString&);

#define LOCTEXT_NAMESPACE "FMapListModule"

void FMapListModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FMapListStyle::Initialize();
	FMapListStyle::ReloadTextures();

	FMapListCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMapListCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FMapListModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMapListModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MapListTabName, FOnSpawnTab::CreateRaw(this, &FMapListModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FMapListTabTitle", "MapList"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMapListModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FMapListStyle::Shutdown();

	FMapListCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MapListTabName);
}

TSharedRef<SDockTab> FMapListModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    // Get the Asset Registry Module
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> MapAssets;

    // Define the content folder path where maps are located
    FString ContentFolder = "/Game/";

    // Use the Asset Query API to find maps (UWorld class) with default search of subclasses (optional)
    AssetRegistry.GetAssetsByClass(TEXT("/Script/Engine.World"), MapAssets, true);

    // Text to display if no maps are found
    FText NoMapsText = FText::FromString("No Maps Found");

    if (MapAssets.Num() > 0) {
        // Create a vertical box to display the list of maps
        TSharedPtr<SWidgetSwitcher> MapListSwitcher = SNew(SWidgetSwitcher);

        // List box to display map names
        FMenuBuilder MenuBuilder(true, nullptr);

        // Loop through all map assets and create menu items and STextBlock for each map name
        for (const FAssetData& MapAsset : MapAssets) {
            FString PackagePath = MapAsset.PackagePath.ToString(); // Get the package path of the map
            FString AssetName = MapAsset.AssetName.ToString(); // Get the asset name of the map

            // Check if the map is within the content folder
            if (PackagePath.StartsWith(ContentFolder, ESearchCase::IgnoreCase)) {
                // Construct the full filesystem path including the asset name and extension
                FString FullMapPath = FPackageName::LongPackageNameToFilename(PackagePath);
                FullMapPath = FPaths::Combine(FullMapPath, AssetName + ".umap"); // Append the asset name with ".umap" extension

                // Add a menu item for each map within the content folder
                MenuBuilder.AddMenuEntry(
                    FText::FromString(AssetName),
                    FText::GetEmpty(),
                    FSlateIcon(),
                    FUIAction(
                        FExecuteAction::CreateLambda([this, FullMapPath] {
                            UE_LOG(LogTemp, Display, TEXT("Printing out the FullMapPath: %s"), *FullMapPath);
                            OpenMap(FullMapPath); // Call OpenMap with the full path of the map
                            })
                    )
                );

                MapListSwitcher->AddSlot()
                    [
                        SNew(STextBlock)
                            .Text(FText::FromString(AssetName))
                    ];
            }
        }

        // Create the final menu widget
        TSharedRef<SWidget> MenuListWidget = MenuBuilder.MakeWidget();

        FText WidgetText = FText::Format(
            LOCTEXT("WindowContentText", "Maps in Project:"),
            FText::FromString(TEXT(""))
        );

        MapListSwitcher->SetActiveWidget(MapListSwitcher->GetWidget(0).ToSharedRef());

        // Vertical box content wrapped inside an SDockTab
        return SNew(SDockTab)
            .Label(FText::FromString("Map List"))
            [
                SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Top)
                    [
                        SNew(STextBlock)
                            .Text(WidgetText)
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Fill)
                    [
                        MenuListWidget
                    ]
            ];
    }
    else {
        // No maps found within the content folder, display text
        return SNew(SDockTab)
            .Label(FText::FromString("Map List"))
            [
                SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Top)
                    [
                        SNew(STextBlock)
                            .Text(NoMapsText)
                    ]
            ];
    }
}

void FMapListModule::OpenMap(FString MapPath)
{
    // Check if we are in editor mode
    if (GEditor)
    {
        // Construct the full path to the map file within the project's content directory
        FString FullMapPath = FPaths::ConvertRelativePathToFull(MapPath);//"C:/Users/Dylan/Documents/Unreal Projects/ProjectIso/Content/FirstPerson/Maps/FirstPersonMap.umap"; //FPaths::ProjectContentDir() / MapPath; // Combine project content directory with the relative map path
        UE_LOG(LogTemp, Display, TEXT("Printing out the MapPath that should be full: %s"), *FullMapPath);

        // Log the map path being loaded
        UE_LOG(LogTemp, Display, TEXT("Attempting to open map: %s"), *FullMapPath);

        // Check if the map file exists at the specified path
        if (FPaths::FileExists(FullMapPath))
        {
            // Get the editor world
            UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();

            if (EditorWorld)
            {
                // Load the map using FEditorFileUtils::LoadMap
                FEditorFileUtils::LoadMap(FullMapPath, false, true);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to obtain a valid editor world."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Map file not found at path: %s"), *FullMapPath);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GEditor is not available. Are we running in editor mode?"));
    }
}

void FMapListModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MapListTabName);
}

void FMapListModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FMapListCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FMapListCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMapListModule, MapList)