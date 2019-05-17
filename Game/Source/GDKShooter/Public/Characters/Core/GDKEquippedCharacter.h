// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Characters/Core/GDKMobileCharacter.h"
#include "Components/EquippedComponent.h"
#include "Components/MetaDataComponent.h"
#include "Weapons/Holdable.h"
#include "Weapons/ITraceProvider.h"

#include "GDKEquippedCharacter.generated.h"


DECLARE_DELEGATE_OneParam(FHoldableSelection, int32);

UCLASS()
class GDKSHOOTER_API AGDKEquippedCharacter : public AGDKMobileCharacter
{
	GENERATED_BODY()

public:
	AGDKEquippedCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
		virtual void StartPrimaryUse();

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UEquippedComponent* EquippedComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMetaDataComponent* MetaDataComponent;

protected:

	UFUNCTION()
		virtual void AttachHoldable(AHoldable* Holdable, FName Socket) const;

	UPROPERTY(EditDefaultsOnly)
		float SprintRecoveryTime = 0.2f;

	UFUNCTION(BlueprintImplementableEvent)
		void OnHeldChanged();

	UFUNCTION()
		void OnEquippedUpdated(AHoldable* NewHoldable);
};
