// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CrossServerPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FIncomingDamageEvent, float, Damage, const struct FDamageEvent&, DamageEvent, AController*, EventInstigator, AActor*, DamageCauser, FVector, Location);

UCLASS()
class GDKSHOOTER_API ACrossServerPawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
		FIncomingDamageEvent IncomingDamage;

	float TakeDamage(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(CrossServer, Reliable)
		void TakeDamageCrossServer(float Damage, const struct FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser, FVector Location);
	
};
