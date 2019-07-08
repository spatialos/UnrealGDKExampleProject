// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/EquippedComponent.h"
#include "Game/GDKMetaData.h"
#include "GameFramework/Actor.h"
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
		
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StartPrimaryUse();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StopPrimaryUse();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StartSecondaryUse();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void StopSecondaryUse();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void ToggleMode();

	UFUNCTION(BlueprintNativeEvent)
	void SetFirstPerson(bool bNewFirstPerson);

	bool bCanBeDropped = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		EHoldableHolsterType Holster = EHoldableHolsterType::HH_Hidden;

	UFUNCTION(BlueprintImplementableEvent)
		void OnMetaDataUpdated();

	void SetMetaData(FGDKMetaData MetaData);

	UFUNCTION(BlueprintNativeEvent)
		void SetIsActive(bool bNewActive);
	
	UFUNCTION(BlueprintCallable)
		FVector EffectSpawnPoint();

	UFUNCTION(BlueprintCallable)
		FName GetActiveSocket() { return ActiveSocket; }

	UFUNCTION(BlueprintCallable)
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

	UPROPERTY(BlueprintReadOnly)
		bool bIsFirstPerson;

	UPROPERTY(BlueprintReadOnly)
		bool bIsActive;	


};
