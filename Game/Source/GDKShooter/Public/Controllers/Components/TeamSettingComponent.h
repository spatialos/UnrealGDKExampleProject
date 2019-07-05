// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/AIModule/Classes/GenericTeamAgentInterface.h"
#include "TeamSettingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamSettingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamSettingComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		FGenericTeamId TeamId = FGenericTeamId::NoTeam;
	
};
