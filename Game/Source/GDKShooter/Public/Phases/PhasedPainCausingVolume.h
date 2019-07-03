// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/PainCausingVolume.h"
#include "IPhaseActivated.h"
#include "PhasedPainCausingVolume.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API APhasedPainCausingVolume : public APainCausingVolume, public IPhaseActivated
{
	GENERATED_BODY()

public:
	APhasedPainCausingVolume() 
	{
		bPainCausing = false; 
	}

	void BeginPlay() override
	{
		if (PostProcessVolume)
		{
			PostProcessVolume->bEnabled = false;
			PostProcessVolume->SetActorLocation(GetActorLocation());
			PostProcessVolume->SetActorScale3D(GetActorScale3D() - FVector(1, 1, 0));
		}
	}

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		int ActivatesInPhase = 255;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
		APostProcessVolume* PostProcessVolume;

	int GetActivatesInPhase_Implementation() { return ActivatesInPhase; }
	
	void Activate_Implementation()
	{
		bPainCausing = true;
		if (PostProcessVolume)
		{
			PostProcessVolume->bEnabled = true;
		} 
	}
	
};
