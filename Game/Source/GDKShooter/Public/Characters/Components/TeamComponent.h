// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"
#include "TeamComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTeamChangedEvent, FGenericTeamId, Team);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamComponent();

	UPROPERTY(BlueprintAssignable)
		FTeamChangedEvent TeamChanged;

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

	UPROPERTY(ReplicatedUsing = OnRep_TeamId, EditDefaultsOnly)
		FGenericTeamId TeamId = FGenericTeamId::NoTeam;

	UFUNCTION()
		void OnRep_TeamId() { TeamChanged.Broadcast(TeamId); };

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
