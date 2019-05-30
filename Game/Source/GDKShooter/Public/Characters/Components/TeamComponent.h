// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/Components/TeamModeComponent.h"
#include "TeamComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamComponent();
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable)
		virtual bool CanDamageActor(AActor* OtherActor);

	UFUNCTION(BlueprintCallable)
		void SetTeam(int32 NewTeamId) { TeamId = NewTeamId; }

	UFUNCTION(BlueprintCallable)
		int32 GetTeam() { return TeamId; }

	//Negative or Zero as a Team Id is not considered a valid team
	UFUNCTION(BlueprintCallable)
		bool HasTeam() { return TeamId > 0; }
		
protected:

	UPROPERTY()
		int32 TeamId;

	UPROPERTY()
		UTeamModeComponent* TeamMode;
	
};
