// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapListCommands.h"

#define LOCTEXT_NAMESPACE "FMapListModule"

void FMapListCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "MapList", "Bring up MapList window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
