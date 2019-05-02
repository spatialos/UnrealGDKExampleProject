// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreUObject.h"

#include "GDKMetaData.generated.h"

// Meta Data sctruct holding information about a player, also passed to a player's inventory items
USTRUCT(BlueprintType)
struct FGDKMetaData {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		int32 Customization;
};
