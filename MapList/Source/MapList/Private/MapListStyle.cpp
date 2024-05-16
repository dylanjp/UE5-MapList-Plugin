// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapListStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FMapListStyle::StyleInstance = nullptr;

void FMapListStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMapListStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FMapListStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("MapListStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FMapListStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("MapListStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("MapList")->GetBaseDir() / TEXT("Resources"));

	Style->Set("MapList.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	// Define custom font and text styles
	Style->Set("MapList.NormalText", FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14)) // Use default engine font with size 14
		.SetColorAndOpacity(FLinearColor::White) // Set text color to white
		.SetShadowOffset(FVector2D(1, 1)) // Add shadow offset
		.SetShadowColorAndOpacity(FLinearColor::Black) // Shadow color
	);

	return Style;
}

void FMapListStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FMapListStyle::Get()
{
	return *StyleInstance;
}
