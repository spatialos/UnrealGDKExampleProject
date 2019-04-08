// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "GDKPlayerScore.generated.h"

// Information about a players performane during a match
USTRUCT(BlueprintType)
struct FPlayerScore {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		int32 PlayerId;

	UPROPERTY(BlueprintReadOnly)
		FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
		int32 Kills;

	UPROPERTY(BlueprintReadOnly)
		int32 Deaths;
};
