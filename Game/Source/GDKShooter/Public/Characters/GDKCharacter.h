// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GDKLogging.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/Character.h"
#include "Characters/Components/HealthComponent.h"
#include "Characters/Components/EquippedComponent.h"
#include "Characters/Components/MetaDataComponent.h"
#include "Characters/Components/GDKMovementComponent.h"
#include "Characters/Components/TeamComponent.h"
#include "Weapons/Holdable.h"
#include "TimerManager.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"
#include "Runtime/AIModule/Classes/Perception/AISightTargetInterface.h"
#include "BlastRuntime/Public/BlastMeshActor.h"
#include "BlastRuntime/Public/TestBlastMesh/TestBlastMeshActor.h"
#include "GDKCharacter.generated.h"

DECLARE_DELEGATE_OneParam(FBoolean, bool);
DECLARE_DELEGATE_OneParam(FHoldableSelection, int32);

enum AIMode_E
{
	AIM_PAUSE = 0,	
	AIM_LOW_FREQUENCY_FIRE,
	AIM_HIGH_FREQUENCY_FIRE,
	AIM_MAX
};

UCLASS()
class GDKSHOOTER_API AGDKCharacter : public ACharacter, public IGenericTeamAgentInterface, public IAISightTargetInterface
{
	GENERATED_BODY()

public:
	AGDKCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UGDKMovementComponent* GDKMovementComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UEquippedComponent* EquippedComponent;
	
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMetaDataComponent* MetaDataComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTeamComponent* TeamComponent;

	UFUNCTION(BlueprintPure)
	float GetRemotePitch() {
		return RemoteViewPitch;
	}

	/** Handles moving forward/backward */
	virtual void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	virtual void MoveRight(float Val);

	UFUNCTION(BlueprintNativeEvent)
	void OnEquippedUpdated(AHoldable* NewHoldable);

	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = NULL) const override;

	UPROPERTY(EditDefaultsOnly)
	TArray<FName> LineOfSightSockets;

	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<ECollisionChannel> LineOfSightCollisionChannel;

	UPROPERTY(EditAnywhere)
	float RagdollLifetime = 5.0f;

	// [client + server] Puts the player in ragdoll mode.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartRagdoll();

	// yunjie: hit event
	UFUNCTION()
	void OnCapsuleCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UFUNCTION()
	void DeleteSelf();

	FTimerHandle DeletionTimer;
	FTimerDelegate DeletionDelegate;
	
public:
	float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(CrossServer, Reliable)
	void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ServerSetAIMode();
	UFUNCTION(Client, Reliable)
	void ClientSetAIMode(int Mode);
	void MulticastAIMode();

	UFUNCTION(BlueprintCallable)
	int GetAIMode() { return AIMode; }

	UFUNCTION(BlueprintCallable)
	float GetBurstDuration();

private:
	void PrintCurrentBlastInfos(const FString& Func = "");
	void ClientPrintCurrentBlastInfos();
	UFUNCTION(Server, Reliable)
	void ServerPrintCurrentBlastInfos();

	UFUNCTION(Server, Reliable)
	void ServerStartTimerToBlast();

	UFUNCTION(Server, Reliable)
	void ServerIncreaseBlastActorCountPerSecond();

	UFUNCTION(Server, Reliable)
	void ServerDecreaseBlastActorCountPerSecond();

	UFUNCTION(Server, Reliable)
	void ServerSpawnBlastActors();

	void SetDebrisLifetime_Quick();
	void SetDebrisLifetime_Normal();
	void SetDebrisLifetime_Forever();

	UFUNCTION(Server, Reliable)
	void ServerSetDebrisLifetime(int32 min, int32 max);

	UFUNCTION(Server, Reliable)
	void ServerPrintBlastStats();

	UFUNCTION()
	void BlastTimerEvent();

	// yunjie: for directly called from client in blueprint
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "GDKCharacter")
	void ServerApplyDamage(ATestBlastMeshActor* BlastActor, FVector Origin, float MinRadius, float MaxRadius, float Damage = 100.0f, float ImpulseStrength = 0.0f, bool bImpulseVelChange = true);


private:

	FTimerHandle BlastTimer;
	FTimerDelegate BlastDelegate;

	int32				BlastActorCountPerSecond = 1;

	TSubclassOf<class ATestBlastMeshActor>					BlastCubeBlueprint;

	int						AIMode = 0;
};
