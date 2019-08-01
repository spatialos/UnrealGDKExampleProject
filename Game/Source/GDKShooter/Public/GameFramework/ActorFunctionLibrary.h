// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActorFunctionLibrary.generated.h"

UCLASS()
class GDKSHOOTER_API UActorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Actor")
		static void RenameDisplayName(AActor* ToRename, const FString NewName);
	
};
