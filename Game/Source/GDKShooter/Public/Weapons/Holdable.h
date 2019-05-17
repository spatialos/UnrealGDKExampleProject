// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/EquippedComponent.h"
#include "Game/GDKMetaData.h"
#include "Holdable.generated.h"

UENUM(BlueprintType)
enum class EHoldableHolsterType : uint8
{
	HH_Hidden 				UMETA(DisplayName = "Hidden"),
	HH_Holster				UMETA(DisplayName = "Holster"),
	HH_Back					UMETA(DisplayName = "Back")
};

UCLASS()
class GDKSHOOTER_API AHoldable : public AActor
{
	GENERATED_BODY()
	
public:	
	AHoldable();
	
	virtual void BeginPlay();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//Locally, is active
	//Who is holding it

	//Can be visible InActive
	//Socket to parent to when Active
	//Socket to parent to when InActive
	//Some way of defining animations
	
public:

	// Returns true only if the primary use can be performed immediately or within a buffer of time
	virtual void StartPrimaryUse() { IsPrimaryUsing = true; }
	virtual void StopPrimaryUse() { IsPrimaryUsing = false; }
	virtual void StartSecondaryUse() { IsSecondaryUsing = true; }
	virtual void StopSecondaryUse() { IsSecondaryUsing = false;  }

	// Return false it was unable to be assigned to the new owner
	virtual bool AssignTo(class UEquippedComponent* NewOwner);

	virtual void SetFirstPerson(bool bNewFirstPerson);

	// Starting weapons would probably be removed when a character dies
	// However weapons that had been picked up might want to persist
	// Same goes for being able to trade/drop weapons
	bool bCanBeDropped = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		EHoldableHolsterType Holster = EHoldableHolsterType::HH_Hidden;

	UFUNCTION(BlueprintImplementableEvent)
		void OnMetaDataUpdated();

	void SetMetaData(FGDKMetaData MetaData);

	virtual void SetIsActive(bool bNewActive);
	
	UFUNCTION(BlueprintCallable)
		FVector EffectSpawnPoint();

	UFUNCTION(BlueprintCallable)
		FName GetActiveSocket() { return ActiveSocket; }

	UFUNCTION()
		virtual void ForceCooldown(float Cooldown) {}

protected:

	UPROPERTY(Transient, BlueprintReadOnly)
		bool IsPrimaryUsing;
	UPROPERTY(Transient, BlueprintReadOnly)
		bool IsSecondaryUsing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		int32 NumberOfModes;

	UPROPERTY(Replicated, BlueprintReadWrite)
		int32 CurrentMode = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName ActiveSocket = "Gun_Transform";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FName EffectSocketName = FName(TEXT("WP_Barrel"));

	USceneComponent* LocationComponent;

	// Visible mesh.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons", meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MetaData)
		FGDKMetaData MetaData;

	UFUNCTION()
		void OnRep_MetaData();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bVisibleWhenInactive;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bCanBeVisibleWhenInactive;
	
	UPROPERTY(BlueprintReadOnly, Transient, ReplicatedUsing = OnRep_Wielder)
		UEquippedComponent* Wielder;

	UFUNCTION()
		virtual void OnRep_Wielder();

	UPROPERTY(BlueprintReadOnly)
		bool bIsFirstPerson;

	UPROPERTY(BlueprintReadOnly)
		bool bIsActive;	
};
