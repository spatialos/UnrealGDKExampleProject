// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GDKCharacter.h"
#include "GDKFPShooterCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API AGDKFPShooterCharacter : public AGDKCharacter
{
	GENERATED_BODY()

public:
	AGDKFPShooterCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void OnEquippedUpdated_Implementation(AHoldable* Holdable) override;

protected:

	/** Name of the FirstPersonMeshComponent. Use this name if you want to prevent creation of the component (with ObjectInitializer.DoNotCreateDefaultSubobject). */
	static FName FirstPersonMeshComponentName;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FirstPersonCamera;

	/** The first person skeletal mesh associated with this Character. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		USkeletalMeshComponent* FirstPersonMesh;
		
	virtual void StartRagdoll_Implementation() override;

};
