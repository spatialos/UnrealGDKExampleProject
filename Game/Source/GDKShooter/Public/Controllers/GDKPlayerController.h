// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Characters/Components/MetaDataComponent.h"
#include "Game/Components/DeathmatchScoreComponent.h"
#include "Game/Components/MatchStateComponent.h"
#include "GDKPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPawnEvent, APawn*, InPawn);

UCLASS(SpatialType)
class GDKSHOOTER_API AGDKPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGDKPlayerController();

	virtual void Tick(float DeltaTime) override;

	FPawnEvent& OnPawn() { return PawnEvent; }

	// Overrides AController::SetPawn, which should be called on the client and server whenever the controller
	// possesses (or unpossesses) a pawn.
	virtual void SetPawn(APawn* InPawn) override;
	
	// [client] Sets whether the cursor is in "UI mode", meaning it is visible and can be moved around the screen,
	// instead of locked, invisible, and used for aiming.v
	UFUNCTION(BlueprintCallable)
		void SetUIMode(bool bIsUIMode);

	// [client] Sets whether we should ignore action input. For this to work properly, the character
	// [client] If true, action input should be ignored. This should be called from the character, or any other object
	// which handles user input.
	bool IgnoreActionInput() const { return bIgnoreActionInput; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UTouchControls> wTouchUI;
	
	UTouchControls* touchUI;

protected:
	UPROPERTY(BlueprintAssignable)
	FPawnEvent PawnEvent;

	UFUNCTION(BlueprintImplementableEvent)
	void OnNewPawn(APawn* InPawn);
		
	/** Death camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* DeathCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	virtual void GetPlayerViewPoint(FVector& out_Location, FRotator& out_Rotation) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LatestPawnYaw;

private:
	// Requests to spawn player pawn and join play
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerTryJoinGame();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerRequestName(const FString& NewPlayerName);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerRequestMetaData(const FGDKMetaData NewMetaData);

	// [server] Causes the character to respawn.
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerRespawnCharacter();

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
