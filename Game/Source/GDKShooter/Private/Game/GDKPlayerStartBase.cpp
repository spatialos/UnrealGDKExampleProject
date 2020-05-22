// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GDKPlayerStartBase.h"

#include "Characters/Components/MetaDataComponent.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKSettings.h"

AGDKPlayerStartBase::AGDKPlayerStartBase(const FObjectInitializer& ObjectInitializer)
	: APlayerStart(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AGDKPlayerStartBase::SpawnCharacter_Implementation(APlayerController* Controller)
{
	if (!Controller->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, Controller] {SpawnCharacter_Implementation(Controller);});
		return;
	}

	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();

	APawn* NewPawn = GameMode->SpawnDefaultPawnFor(Controller, this);

	Controller->Possess(NewPawn);

	if (UMetaDataComponent* StateMetaData = Cast<UMetaDataComponent>(Controller->PlayerState->GetComponentByClass(UMetaDataComponent::StaticClass())))
	{
		if (UMetaDataComponent* MetaData = Cast<UMetaDataComponent>(NewPawn->GetComponentByClass(UMetaDataComponent::StaticClass())))
		{
			MetaData->SetMetaData(StateMetaData->GetMetaData());
		}
	}
}
