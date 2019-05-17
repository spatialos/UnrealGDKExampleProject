// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Game/GDKSessionProgress.h"
#include "Game/GDKPlayerScore.h"
#include "Blueprint/UserWidget.h"
#include "GDKWidget.generated.h"

/**
 * UMG Widget exposing multiple Events relating to Pawn, Player and Game state.
 */
UCLASS()
class GDKSHOOTER_API UGDKWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeConstruct() override;

protected:

	UPROPERTY(BlueprintReadOnly)
		APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly)
		bool bListenersAdded;

	UFUNCTION()
		void OnPawn(APawn* InPawn);

	// Called whenever the current health value is updated
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnHealthUpdated(float CurrentHealth, float MaxHealth);

	// Called whenever the current armour value is updated
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnArmourUpdated(float CurrentArmour, float MaxArmour);

	// Called whenever the local player dies
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnDeath(const FString& KillerName, int32 KillerId);

	// Called whenever the local player scores a killing blow on another player
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnKill(const FString& VictimName,int32 VictimId);

	// Called when the equipped weapon is changed
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnWeaponChanged(AWeapon* Weapon);

	// Called when the local player starts or stops aiming
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnAimingUpdated(bool bIsAiming);

	// Called when the game state RepNotifies a new scoreboard
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnPlayerScoresUpdated(const TArray<FPlayerScore>& Scores);

	// Called each time the game state timer ticks or changes session state
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnTimerUpdated(EGDKSessionProgress SessionProgress, int SecondsRemaining);

	// Called each time a shot is fired by the local player
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnShot(AWeapon* Weapon, bool Hit);

	// Called each time the server RepNotifies a new number of players connected
	UFUNCTION(BlueprintImplementableEvent, Category = "GDK")
		void OnPlayerCountUpdated(int PlayerCount);

	// Function to call to ClientTravel to the TargetMap
	UFUNCTION(BlueprintCallable, Category="GDK")
		void LeaveGame(const FString& TargetMap);
};
