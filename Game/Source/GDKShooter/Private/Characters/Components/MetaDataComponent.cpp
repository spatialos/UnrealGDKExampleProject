// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "MetaDataComponent.h"
#include "UnrealNetwork.h"
#include "GDKLogging.h"

UMetaDataComponent::UMetaDataComponent()
{
	bReplicates = true;
}

void UMetaDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMetaDataComponent, MetaData);
	DOREPLIFETIME(UMetaDataComponent, bMetaDataAvailable);
}

void UMetaDataComponent::SetMetaData(FGDKMetaData NewMetaData)
{
	MetaData = NewMetaData;
	bMetaDataAvailable = true;
	MetaDataUpdated.Broadcast(MetaData);
}

void UMetaDataComponent::OnRep_MetaData()
{
	MetaDataUpdated.Broadcast(MetaData);
}
