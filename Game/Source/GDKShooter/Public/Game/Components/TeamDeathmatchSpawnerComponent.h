// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TeamDeathmatchSpawnerComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTeamDeathmatchSpawnerComponent, Log, All)

USTRUCT()
struct FTeamAssignment
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	int32 TeamId;

	UPROPERTY()
	int32 PlayerNum;

public:
	FTeamAssignment(int32 Id = 0, int32 Num = 0) : TeamId(Id), PlayerNum(Num) {}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamDeathmatchSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamDeathmatchSpawnerComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void SetTeams(TArray<FGenericTeamId> TeamIds);

	UFUNCTION(BlueprintCallable)
	void RequestSpawn(APlayerController* Controller);

	UFUNCTION(BlueprintCallable)
	void PlayerDisconnected(APlayerController* Controller);

	// When Enabled, will spawn players at players starts that have a UTeamComponent which matches the players team.
	UPROPERTY(EditDefaultsOnly)
	bool bUseTeamPlayerStarts;

	// When Enabled, will shuffle player starts to avoid 2 consecutive spawns being next to each other.
	UPROPERTY(EditDefaultsOnly)
	bool bShufflePlayerStarts;

protected:
	int32 GetSmallestTeam();
	void ShufflePlayerStartArray(TArray<class APlayerStart*> Array);

	class APlayerStart* GetNextTeamPlayerStart(FGenericTeamId Team);
	class APlayerStart* GetNextPlayerStart();

	UPROPERTY(Replicated)
	TArray<class APlayerStart*> PlayerStarts;

	UPROPERTY(Replicated)
	TArray<class APlayerStart*> TeamPlayerStarts;

	UPROPERTY(Replicated)
	//TMap<int32, int32> TeamAssignments;
	TArray<FTeamAssignment> TeamAssignments;

	TMap<APlayerController*, int32> SpawnedPlayers;
	TMap<FGenericTeamId, int32> NextTeamPlayerStart;
	int32 NextPlayerStart;
};
