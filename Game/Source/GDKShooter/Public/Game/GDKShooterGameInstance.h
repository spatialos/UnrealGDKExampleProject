// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "EngineClasses/SpatialGameInstance.h"

#include "GDKShooterGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGDKShooterGameInstance, Log, All);

USTRUCT(BlueprintType)
struct FNetworkFailureInfo 
{
	GENERATED_BODY()

	FNetworkFailureInfo();
	FNetworkFailureInfo(ENetworkFailure::Type InNetworkFailureType, const FString& InReason);

	UPROPERTY(BlueprintReadOnly)
	bool NetworkFailureOccurred;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<ENetworkFailure::Type> NetworkFailureType;
	
	UPROPERTY(BlueprintReadOnly)
	FString Reason;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkFailure, FNetworkFailureInfo, NetworkFailureInfo);

UCLASS()
class GDKSHOOTER_API UGDKShooterGameInstance : public USpatialGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	void HandleOnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type NetworkFailureType, const FString& Reason);

	UFUNCTION(BlueprintCallable)
	void GetNetworkFailureInfo(FNetworkFailureInfo& NetworkFailureInfo);

	UPROPERTY(BlueprintAssignable)
	FOnNetworkFailure OnNetworkFailure;

private:
	UPROPERTY()
	FNetworkFailureInfo NetworkFailureInfo;
};
