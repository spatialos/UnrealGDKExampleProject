// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "GDKShooterFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API UGDKShooterFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Teams")
	static void SetGenericTeamId(AActor* Actor, FGenericTeamId NewTeamId);

	UFUNCTION(BlueprintPure, Category = "Teams")
	static FGenericTeamId GetGenericTeamId(AActor* Actor);
};
