// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Game/GDKShooterGameInstance.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY(LogGDKShooterGameInstance);

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
	UE_LOG(LogGDKShooterGameInstance, Log, TEXT("Network failure due to: %s"), *Reason);

	NetworkFailureInfo = FNetworkFailureInfo(NetworkFailureType, Reason);

	OnNetworkFailure.Broadcast(NetworkFailureInfo);
}

void UGDKShooterGameInstance::GetNetworkFailureInfo(FNetworkFailureInfo& OutNetworkFailureInfo)
{
	OutNetworkFailureInfo = NetworkFailureInfo;
}

