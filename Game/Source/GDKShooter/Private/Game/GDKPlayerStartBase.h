// Copyright (c) Improbable Worlds Ltd, All Rights Reserved#
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "GDKPlayerStartBase.generated.h"

class AGDKPlayerController;

UCLASS(Blueprintable)
class GDKSHOOTER_API AGDKPlayerStartBase : public APlayerStart
{
	GENERATED_BODY()
public:
	AGDKPlayerStartBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(CrossServer, Reliable, BlueprintCallable, Category = "Spatial")
	void SpawnCharacter(APlayerController* Controller);
};
