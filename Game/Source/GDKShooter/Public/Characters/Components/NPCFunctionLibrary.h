// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NPCFunctionLibrary.generated.h"

UCLASS()
class GDKSHOOTER_API UNPCFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "AI")
		static bool TargetLocation(const APawn* SelfAsPawn, const AActor* Target, FVector& Location);
	
};
