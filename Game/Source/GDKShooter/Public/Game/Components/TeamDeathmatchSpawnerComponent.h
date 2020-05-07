// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TeamDeathmatchSpawnerComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTeamDeathmatchSpawnerComponent, Log, All)

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

	// When Enabled, will spawn players at players starts that have a UTeamComponent which matches the players team.
	UPROPERTY(EditDefaultsOnly)
	bool bUseTeamPlayerStarts;

protected:
	int32 GetSmallestTeam();

	class APlayerStart* GetNextTeamPlayerStart(FGenericTeamId Team);
	class APlayerStart* GetNextPlayerStart();

	TArray<class APlayerStart*> PlayerStarts;
	TMap<int32, int32> TeamAssignments;
	TMap<APlayerController*, int32> SpawnedPlayers;
	TMap<FGenericTeamId, int32> NextTeamPlayerStart;
	int32 NextPlayerStart;
};
