// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Game/GDKPlayerScore.h"
#include "GDKGameState.generated.h"

// Events for listening to game state updates
DECLARE_EVENT_OneParam(AGDKGameState, FScoreChangeEvent, const TArray<FPlayerScore>&);
DECLARE_EVENT_OneParam(AGDKGameState, FPlayerCountEvent, int);

UCLASS(SpatialType = Singleton)
class GDKSHOOTER_API AGDKGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGDKGameState();

	// Adds a player to the game's score data with empty stats.
	void AddPlayer(const int32 Actor, const FString& Player);

	// Adds a death (and a corresponding kill, if necessary) to the game's score data.
	void AddDeath(const int32 Killer, const int32 Victim);

	FScoreChangeEvent& OnScoreUpdated() { return ScoreEvent; }

	FPlayerCountEvent& OnPlayerCountUpdated() { return PlayerCountEvent; }

	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayers)
		int ConnectedPlayers;

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	TArray<FPlayerScore>& PlayerScores() { return PlayerScoreArray; }

protected:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	UFUNCTION()
		void OnRep_ConnectedPlayers();

	UFUNCTION()
		void OnRep_PlayerScores();

	UFUNCTION(NetMulticast, Unreliable)
		void MulticastNotifyKill(int32 KillerId, const FString& KillerName, int32 VictimId, const FString& VictimName);

	// Retrieves the FTeamScore value for a given team, or nullptr if the team is invalid (or the scores haven't been initialized yet).
	FPlayerScore* GetScoreForPlayer(int32 Player);

	// Actually adds the player.
	void AddPlayerInternal(const int32 Player, const FString& PlayerName);

	// List of teams' scores, including top player lists.
	UPROPERTY(ReplicatedUsing = OnRep_PlayerScores)
		TArray<FPlayerScore> PlayerScoreArray;
	
	// A map from player name to score, to make it easier to refer to players.
	UPROPERTY()
		TMap<int32, int32> PlayerScoreMap;

	FScoreChangeEvent ScoreEvent;
	FPlayerCountEvent PlayerCountEvent;
};
