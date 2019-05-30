// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeamModeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UTeamModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTeamModeComponent();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		int32 MaxTeamSize;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		int32 MaxTeamCount;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
		bool bAllowFriendlyFire;
	
};
