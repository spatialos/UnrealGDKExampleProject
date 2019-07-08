// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "DeathmatchSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UDeathmatchSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDeathmatchSpawnerComponent();

	UFUNCTION(BlueprintCallable)
		void RequestSpawn(APlayerController* Controller);

	UFUNCTION(BlueprintCallable)
		void EnableSpawning() { bSpawningEnabled = true; }

	UFUNCTION(BlueprintCallable)
		void DisableSpawning() { bSpawningEnabled = false; }

protected:
	AActor* GetSpawnPoint(APlayerController* Controller);

	void SpawnCharacter(APlayerController* Controller);

	bool bSpawningEnabled;
};
