// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeathmatchScoreComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TeamDeathmatchScoreComponent.generated.h"

// Information about a teams performance during a match
USTRUCT(BlueprintType)
struct FTeamScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGenericTeamId TeamId;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalKills;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalDeaths;

	UPROPERTY(BlueprintReadOnly)
	TArray<FPlayerScore> PlayerScores;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTeamScoreChangeEvent, const TArray<FTeamScore>&, LatestScores);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamDeathmatchScoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTeamDeathmatchScoreComponent();

	UFUNCTION(BlueprintCallable)
	void RecordKill(APlayerState* KillerState, APlayerState* VictimState);

	UFUNCTION(BlueprintCallable)
	void RecordNewPlayer(APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable)
	void RemovePlayer(APlayerState* PlayerState);

	UFUNCTION(BlueprintPure)
	TArray<FTeamScore>& TeamScores() { return TeamScoreArray; }

	UPROPERTY(BlueprintAssignable)
	FTeamScoreChangeEvent ScoreEvent;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_TeamScores();

	UPROPERTY(ReplicatedUsing = OnRep_TeamScores)
	TArray<FTeamScore> TeamScoreArray;

	UPROPERTY()
	TMap<uint8, int32> TeamScoreMap;

	UPROPERTY()
	TMap<int32, int32> PlayerScoreMap;
};
