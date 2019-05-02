// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreUObject.h"

// States of a session, to match the schema of the deployment manager
UENUM(BlueprintType)
enum class EGDKSessionProgress : uint8
{
	Lobby				UMETA(DisplayName = "Lobby"),
	Running				UMETA(DisplayName = "Running"),
	Results				UMETA(DisplayName = "Results"),
	Finished			UMETA(DisplayName = "Finished"),
};
