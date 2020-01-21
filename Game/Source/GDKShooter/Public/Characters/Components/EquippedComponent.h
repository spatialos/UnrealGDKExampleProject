// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetaDataComponent.h"
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
	UFUNCTION(BlueprintCallable)
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

	UFUNCTION(BlueprintCallable)
		void QuickToggle();

	UFUNCTION(BlueprintCallable)
		void ToggleMode();

	UFUNCTION(BlueprintCallable)
		void ScrollUp();

	UFUNCTION(BlueprintCallable)
		void ScrollDown();

	bool HasHoldableAtIndex(int32 Index);

	UPROPERTY(BlueprintAssignable)
		FHoldableUpdated HoldableUpdated;

	UFUNCTION(BlueprintCallable)
		bool Grant(AHoldable* NewHoldable);

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

	UFUNCTION()
		bool HasAnyEmptySlots();

	UFUNCTION()
		int GetNextAvailableSlot();

	UFUNCTION()
		bool AlreadyHas(AHoldable* NewHoldable);

// Using Logic
public:
	UFUNCTION()
		void SetIsSprinting(bool bNewSprinting) { bIsSprinting = bNewSprinting; }

	UFUNCTION()
		void BlockUsing(bool bBlock);

	UFUNCTION(BlueprintCallable)
		void StartPrimaryUse();
	UFUNCTION(BlueprintCallable)
		void StopPrimaryUse();
	UFUNCTION(BlueprintCallable)
		void StartSecondaryUse();
	UFUNCTION(BlueprintCallable)
		void StopSecondaryUse();

	UFUNCTION()
		void ForceCooldown(float Cooldown);

	UPROPERTY(EditDefaultsOnly)
		float SprintRecoveryTime = 0.2f;

protected:

	// Is using holdables blocked by e.g. Menus being open
	bool bBlockUsing;
	// Are we currently busy using an item, used to e.g. block sprinting
	bool bIsBusy;
	// Do we think we are sprinting
	// Should we apply the sprinting cooldown when going to use a holdable
	bool bIsSprinting;

	int32 LastCachedIndex = -1;
	int32 CurrentCachedIndex = -1;

};
