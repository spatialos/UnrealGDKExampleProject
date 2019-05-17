// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Characters/Components/MetaDataComponent.h"
#include "Game/Components/DeathmatchScoreComponent.h"
#include "Game/Components/MatchStateComponent.h"
#include "GDKPlayerController.generated.h"

DECLARE_EVENT_TwoParams(AGDKPlayerController, FKillNotificationEvent, const FString&, int32);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPawnEvent, APawn*, InPawn);

UCLASS(SpatialType)
class GDKSHOOTER_API AGDKPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGDKPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	FPawnEvent& OnPawn() { return PawnEvent; }

	FKillNotificationEvent& OnKillNotification() { return KillNotification; }
	FKillNotificationEvent& OnKilledNotification() { return KilledNotification; }

	// Overrides AController::SetPawn, which should be called on the client and server whenever the controller
	// possesses (or unpossesses) a pawn.
	virtual void SetPawn(APawn* InPawn) override;

	// [server] Tells the controller that it's time for the player to die, and sets up conditions for respawn.
	// @param Killer  The player who killed me. Can be null if it wasn't a player who dealt the damage that killed me.
	UFUNCTION(BlueprintCallable)
		void KillCharacter(const class AActor* Killer);

	// [client] Sets whether the cursor is in "UI mode", meaning it is visible and can be moved around the screen,
	// instead of locked, invisible, and used for aiming.v
	UFUNCTION(BlueprintCallable)

	// [client] Sets whether we should ignore action input. For this to work properly, the character
	// [client] If true, action input should be ignored. This should be called from the character, or any other object
	// which handles user input.
	bool IgnoreActionInput() const { return bIgnoreActionInput; }

	// [client] Sets the player-choice data (name, team, etc) and requests to spawn player pawn and join play.
	UFUNCTION(BlueprintCallable)
		void TryJoinGame();

	UFUNCTION(BlueprintCallable)
		void RequestRespawn();

	UFUNCTION(Client, unreliable)
		void InformOfKill(const FString& VictimName, int32 VictimId);
	UFUNCTION(Client, unreliable)
		void InformOfDeath(const FString& KillerName, int32 KillerId);

protected:
	UPROPERTY(BlueprintAssignable)
		FPawnEvent PawnEvent;

	FKillNotificationEvent KillNotification;
	FKillNotificationEvent KilledNotification;
		
	/** Death camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* DeathCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	virtual void GetPlayerViewPoint(FVector& out_Location, FRotator& out_Rotation) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		float LatestPawnYaw;

	UPROPERTY(EditDefaultsOnly)
		bool bAutoConnect;

private:
	// Sets the player-choice data (name, team, etc) and requests to spawn player pawn and join play
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerTryJoinGame();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerRequestName(const FString& NewPlayerName);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
		void ServerRequestMetaData(const FGDKMetaData NewMetaData);

	// [server] Causes the character to respawn.
	UFUNCTION(Server, Reliable, WithValidation)
		void RespawnCharacter();

	// Gets a default player name based upon the worker's ID.
	// Generates a GUID if we're not running on a SpatialOS worker.
	FString GetDefaultPlayerName();

	// If true, action input should be ignored.
	// Default value is false.
	bool bIgnoreActionInput;

	// Time for which to keep the character's body around before deleting it.
	UPROPERTY(EditDefaultsOnly, Category = "Respawn")
		float DeleteCharacterDelay;

	FTimerHandle RespawnTimerHandle;


};
