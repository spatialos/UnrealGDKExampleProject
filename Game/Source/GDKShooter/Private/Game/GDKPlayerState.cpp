// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/GDKPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "SpatialNetDriver.h"


void AGDKPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGDKPlayerState, MetaData);
}
