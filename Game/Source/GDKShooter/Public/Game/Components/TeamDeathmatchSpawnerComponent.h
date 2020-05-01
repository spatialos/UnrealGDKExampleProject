// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamDeathmatchSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamDeathmatchSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamDeathmatchSpawnerComponent();

	UFUNCTION(BlueprintCallable)
	void RequestSpawn(APlayerController* Controller);

	UPROPERTY(EditDefaultsOnly)
	int32 NumTeams = 2;

protected:
	virtual void BeginPlay() override;		

	int32 GetSmallestTeam();

	TArray<class APlayerStart*> PlayerStarts;
	TMap<int32, int32> TeamAssignments;
	TMap<APlayerController*, int32> SpawnedPlayers;
	int32 NextPlayerStart = 0;
};
