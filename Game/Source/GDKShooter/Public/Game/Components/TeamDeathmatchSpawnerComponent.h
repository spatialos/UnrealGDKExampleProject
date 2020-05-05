// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TeamDeathmatchSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamDeathmatchSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamDeathmatchSpawnerComponent();

	UFUNCTION(BlueprintCallable)
	void SetTeams(TArray<FGenericTeamId> TeamIds);

	UFUNCTION(BlueprintCallable)
	void RequestSpawn(APlayerController* Controller);

	UFUNCTION(BlueprintCallable)
	void PlayerDisconnected(APlayerController* Controller);

protected:
	int32 GetSmallestTeam();

	TArray<class APlayerStart*> PlayerStarts;
	TMap<int32, int32> TeamAssignments;
	TMap<APlayerController*, int32> SpawnedPlayers;
	int32 NextPlayerStart = 0;
};
