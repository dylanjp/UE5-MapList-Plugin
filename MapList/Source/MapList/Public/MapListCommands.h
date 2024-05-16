// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MapListStyle.h"

class FMapListCommands : public TCommands<FMapListCommands>
{
public:

	FMapListCommands()
		: TCommands<FMapListCommands>(TEXT("MapList"), NSLOCTEXT("Contexts", "MapList", "MapList Plugin"), NAME_None, FMapListStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};