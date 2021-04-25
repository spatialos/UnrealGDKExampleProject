// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/NpcCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Net/UnrealNetwork.h"
#include "GDKLogging.h"
#include "Controllers/GDKPlayerController.h"
#include "AIModule/Classes/AIController.h"
#include "Controllers/Components/ControllerEventsComponent.h"
#include "Weapons/Holdable.h"
#include "GameFramework/C10KGameState.h"


ANpcCharacterBase::ANpcCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ANpcCharacterBase::OnAuthorityGained()
{
	Super::OnAuthorityGained();
}
