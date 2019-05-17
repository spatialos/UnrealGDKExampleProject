// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/GDKMetaData.h"
#include "EquippedComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHoldableUpdated, AHoldable*, NewHoldable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBusyUpdated, bool, bIsBusy);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UEquippedComponent : public UActorComponent
{
	GENERATED_BODY()

// Unreal Logic
public:	
	UEquippedComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

// Starter Templates
public:
	UFUNCTION()
		virtual void SpawnStarterTemplates(FGDKMetaData NewMetaData);

protected:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		int32 HoldableCapacity = 4;

	// Holdables available to the player on spawn
	UPROPERTY(EditDefaultsOnly, Category = "Holdables")
		TArray<TSubclassOf<AHoldable>> StarterTemplates;

// Held Items
public:
	UFUNCTION(BlueprintPure)
		AHoldable* CurrentlyHeldItem() const;

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerRequestEquip(int32 Index);

	// Called in BeginPlay of the Holdable when it realises it exists
	// Not currently called when a holdable is given a new wielder
	UFUNCTION()
		void InformWielderOfWielded(class AHoldable* WieldedHoldable);

	UPROPERTY(BlueprintAssignable)
		FHoldableUpdated HoldableUpdated;

protected:
	UFUNCTION()
		void OnRep_HeldUpdate();

	UFUNCTION()
		virtual void LocallyActivate(AHoldable* Holdable);

	UPROPERTY()
		AHoldable* LocallyActiveHoldable;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing = OnRep_HeldUpdate)
		int CurrentHeldIndex;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing = OnRep_HeldUpdate)
		TArray<AHoldable*> HeldItems;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
		bool bHeldItemsInitialised;

// Using Logic
public:
	UFUNCTION()
		bool IsBusy() { return bIsBusy; }
	UFUNCTION()
		void SetBusy(bool bNewBusy);

	UFUNCTION()
		void BlockUsing(bool bBlock);

	UFUNCTION()
		void StartPrimaryUse();
	UFUNCTION()
		void StopPrimaryUse();
	UFUNCTION()
		void StartSecondaryUse();
	UFUNCTION()
		void StopSecondaryUse();

	UFUNCTION()
		void ForceCooldown(float Cooldown);

	UPROPERTY(BlueprintAssignable)
		FBusyUpdated BusyUpdated;

protected:

	bool bBlockUsing;
	bool bIsAiming;
	bool bIsBusy;
	bool bIsCrouching;
	bool bCanUseHoldables;


};
