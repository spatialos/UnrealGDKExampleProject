// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Game/GDKMetaData.h"
#include "Game/GDKPlayerScore.h"
#include "Game/GDKSessionProgress.h"
#include "GDKPlayerController.generated.h"

UENUM(BlueprintType)
enum class EGDKMenu : uint8
{
	None				UMETA(DisplayName = "None"),
	Menu				UMETA(DisplayName = "Menu"),
	Scores				UMETA(DisplayName = "Scores"),
};

UENUM(BlueprintType)
enum class EGDKControllerState : uint8
{
	PreCharacter		UMETA(DisplayName = "PreCharacter"),
	PendingCharacter	UMETA(DisplayName = "PendingCharacter"),
	InProgress			UMETA(DisplayName = "InProgress"),
	Finished			UMETA(DisplayName = "Finished"),
};

UENUM(BlueprintType)
enum class EGDKCharacterState : uint8
{
	Alive			UMETA(DisplayName = "Alive"),
	Dead			UMETA(DisplayName = "Dead"),
	PendingRespawn	UMETA(DisplayName = "PendingRespawn"),
};

UCLASS(SpatialType)
class GDKSHOOTER_API AGDKPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGDKPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMenuEvent, EGDKMenu, CurrentMenu);
	UPROPERTY(BlueprintAssignable)
		FMenuEvent OnMenuChanged;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FControllerStateEvent, EGDKControllerState, CurrentState);
	UPROPERTY(BlueprintAssignable)
		FControllerStateEvent OnControllerState;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCharacterStateEvent, EGDKCharacterState, CurrentState);
	UPROPERTY(BlueprintAssignable)
		FCharacterStateEvent OnCharacterState;
	

	DECLARE_EVENT_TwoParams(AGDKPlayerController, FValueChangedEvent, int32, int32);
	FValueChangedEvent& OnHealthUpdated() { return HealthChangedEvent; }
	FValueChangedEvent& OnArmourUpdated() { return ArmourChangedEvent; }

	DECLARE_EVENT_OneParam(AGDKPlayerController, FBooleanChangedEvent, bool);
	FBooleanChangedEvent& OnAimingUpdated() { return AimingChangedEvent; }

	DECLARE_EVENT_TwoParams(AGDKPlayerController, FShotEvent, AWeapon*, bool);
	FShotEvent& OnShot() { return ShotEvent; }

	DECLARE_EVENT_OneParam(AGDKPlayerController, FKillNotificationEvent, const FString&);
	FKillNotificationEvent& OnKillNotification() { return KillNotification; }
	FKillNotificationEvent& OnKilledNotification() { return KilledNotification; }

	void UpdateHealthUI(int32 NewHealth, int32 MaxHealth);
	void UpdateArmourUI(int32 NewArmour, int32 MaxArmour);

	DECLARE_EVENT_OneParam(AGDKPlayerController, FWeaponEvent, AWeapon*);
	FWeaponEvent& OnWeaponChanged() { return WeaponNotification; }

	// Overrides AController::SetPawn, which should be called on the client and server whenever the controller
	// possesses (or unpossesses) a pawn.
	virtual void SetPawn(APawn* InPawn) override;

	// [server] Tells the controller that it's time for the player to die, and sets up conditions for respawn.
	// @param Killer  The player who killed me. Can be null if it wasn't a player who dealt the damage that killed me.
	void KillCharacter(const class AGDKCharacter* Killer);

	// [client] Sets whether the cursor is in "UI mode", meaning it is visible and can be moved around the screen,
	// instead of locked, invisible, and used for aiming.v
	void SetUIMode();
	void SetUIMode(bool bIsUIMode, bool bAllowMovement = false);

	// [client] Sets whether we should ignore action input. For this to work properly, the character
	// must check the result of IgnoreActionInput before applying any action inputs.
	void SetIgnoreActionInput(bool bIgnoreInput) { bIgnoreActionInput = bIgnoreInput; }

	// [client] If true, action input should be ignored. This should be called from the character, or any other object
	// which handles user input.
	bool IgnoreActionInput() const { return bIgnoreActionInput; }

	// [client] Sets the player-choice data (name, team, etc) and requests to spawn player pawn and join play.
	UFUNCTION(BlueprintCallable)
		void TryJoinGame(const FString& NewPlayerName, const FGDKMetaData MetaData);

	UFUNCTION(BlueprintCallable)
		void RequestRespawn();

	void UpdatePlayerScores(const TArray<FPlayerScore>& PlayerScores);

	UFUNCTION(Client, unreliable)
		void InformOfKill(const FString& VictimName);
	UFUNCTION(Client, unreliable)
		void InformOfDeath(const FString& KillerName);

protected:
	virtual void SetupInputComponent() override;

	bool bHasRequetsedPlayer;
	bool bHasBeenGrantedPlayer;

	UFUNCTION(BlueprintImplementableEvent)
		void OnAimingChanged(bool bIsAiming, float AimRotationSpeed);

	virtual void OnCharacterShot(AWeapon* Weapon, bool Hit);

	UFUNCTION(BlueprintImplementableEvent)
		void OnShot(AWeapon* Weapon, bool Hit);

	virtual void ChooseNewSpawnPoint();

	FValueChangedEvent HealthChangedEvent;
	FValueChangedEvent ArmourChangedEvent;

	FBooleanChangedEvent AimingChangedEvent;

	FShotEvent ShotEvent;

	FKillNotificationEvent KillNotification;
	FKillNotificationEvent KilledNotification;

	FWeaponEvent WeaponNotification;

	UPROPERTY(BlueprintReadOnly)
		EGDKMenu CurrentMenu = EGDKMenu::None;
	UPROPERTY(BlueprintReadOnly)
		EGDKCharacterState CurrentCharacterState = EGDKCharacterState::Alive;
	UPROPERTY(BlueprintReadOnly)
		EGDKControllerState CurrentControllerState = EGDKControllerState::PreCharacter;

private:
	// Sets the player-choice data (name, team, etc) and requests to spawn player pawn and join play
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerTryJoinGame(const FString& NewPlayerName, const FGDKMetaData MetaData);

	// [client] Informs the invoking client whether the join request suceeded or failed
	UFUNCTION(Client, Reliable)
		void ClientJoinResults(const bool bJoinSucceeded);

	// [server] Causes the character to respawn.
	UFUNCTION(Server, Reliable, WithValidation)
		void RespawnCharacter();

	void AimingChanged(bool bIsAiming, float AimRotationSpeed);

	void WeaponChanged(AWeapon* Weapon);

	bool bGameFinished;

	// Gets a default player name based upon the worker's ID.
	// Generates a GUID if we're not running on a SpatialOS worker.
	FString GetDefaultPlayerName();

	// If true, action input should be ignored.
	// Default value is false.
	bool bIgnoreActionInput;

	// Time for which to keep the character's body around before deleting it.
	UPROPERTY(EditDefaultsOnly, Category = "Respawn")
		float DeleteCharacterDelay;

	UFUNCTION()
		void TimerUpdated(EGDKSessionProgress SessionProgress, int SessionTimer);

	FTimerHandle RespawnTimerHandle;

	void SetControllerState(EGDKControllerState NewState);
	void SetCharacterState(EGDKCharacterState NewState);
	void SetMenu(EGDKMenu NewMenu);

	void ShowScoreboard();;
	void HideScoreboard();
	void ToggleMenu();


};
