// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"
#include "TeamComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamComponent();

	UFUNCTION(BlueprintPure)
		virtual bool CanDamageActor(AActor* OtherActor);

	UFUNCTION(BlueprintCallable)
		void SetTeam(FGenericTeamId NewTeamId) { TeamId = NewTeamId; }

	UFUNCTION(BlueprintPure)
		FGenericTeamId GetTeam() { return TeamId; }

	//Negative or Zero as a Team Id is not considered a valid team
	UFUNCTION(BlueprintPure)
		bool HasTeam() { return TeamId != FGenericTeamId::NoTeam; }

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bAllowFriendlyFire;

protected:

	UPROPERTY(Replicated)
		FGenericTeamId TeamId = FGenericTeamId::NoTeam;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
