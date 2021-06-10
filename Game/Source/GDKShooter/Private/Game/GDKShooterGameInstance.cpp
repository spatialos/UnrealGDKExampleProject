// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Game/GDKShooterGameInstance.h"
#include "Logging/LogMacros.h"
#include "Utils/SpatialStatics.h"

FNetworkFailureInfo::FNetworkFailureInfo()
	: NetworkFailureOccurred(false)
	, NetworkFailureType(-1)
	, Reason("")
{
}

FNetworkFailureInfo::FNetworkFailureInfo(ENetworkFailure::Type InNetworkFailureType, const FString& InReason)
	: NetworkFailureOccurred(true)
	, NetworkFailureType(InNetworkFailureType)
	, Reason(InReason)
{
}

void UGDKShooterGameInstance::Init()
{
	Super::Init();

	GetEngine()->OnNetworkFailure().AddUObject(this, &UGDKShooterGameInstance::HandleOnNetworkFailure);
}

void UGDKShooterGameInstance::HandleOnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type NetworkFailureType, const FString& Reason)
{
	USpatialStatics::PrintStringSpatial(World, FString::Printf(TEXT("Network Failure: %s"), *Reason), false);

	NetworkFailureInfo = FNetworkFailureInfo(NetworkFailureType, Reason);

	OnNetworkFailure.Broadcast(NetworkFailureInfo);
}

void UGDKShooterGameInstance::GetNetworkFailureInfo(FNetworkFailureInfo& OutNetworkFailureInfo)
{
	OutNetworkFailureInfo = NetworkFailureInfo;
}

