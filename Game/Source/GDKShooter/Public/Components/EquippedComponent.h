// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/GDKMetaData.h"
#include "EquippedComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHoldableUpdated, AHoldable*, NewHoldable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBusyUpdated, bool, bIsBusy);

// UObject class used as a handle when other classes want to block using
UCLASS()
class UBlockingHandle : public UObject
{
	GENERATED_BODY()
};

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

protected:
	UFUNCTION()
		void OnRep_HeldUpdate();

	UFUNCTION()
		virtual void LocallyActivate(AHoldable* Holdable);

	UPROPERTY()
		AHoldable* LocallyActiveHoldable;

	// Default value of -1 suggest no item is being held
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing = OnRep_HeldUpdate)
		int CurrentHeldIndex = -1;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, ReplicatedUsing = OnRep_HeldUpdate)
		TArray<AHoldable*> HeldItems;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Replicated)
		bool bHeldItemsInitialised;

// Using Logic
public:
	UFUNCTION()
		void SetIsSprinting(bool bNewSprinting) { bIsSprinting = bNewSprinting; }

	UFUNCTION()
		UObject* BlockUsing();
	UFUNCTION()
		void UnblockUsing(UObject* BlockingHandle);

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

	TArray<UObject*> BlockingObjects;

	int32 LastCachedIndex = -1;
	int32 CurrentCachedIndex = -1;

};
