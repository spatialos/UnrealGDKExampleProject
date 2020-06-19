// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "TeamSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTeamSpawnerComponent();

	UFUNCTION(BlueprintCallable)
	void RequestSpawn(APlayerController* Controller);

	UPROPERTY(EditDefaultsOnly)
	int32 TeamCapacity = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bAllowRespawning = true;

protected:
	void BeginPlay() override;

	APlayerStartPIE* PlayerStartPIE;
	TMap<int32, APlayerStart*> TeamStartPoints;
	TMap<int32, int32> TeamAssignments;
	TMap<APlayerController*, int32> SpawnedPlayers;

	int32 CurrentTeamPointer = 0;
	int32 StartingTeamId = 1;
	int32 GetAvailableTeamId();
};
