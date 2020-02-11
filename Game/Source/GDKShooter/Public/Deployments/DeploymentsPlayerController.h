// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include <WorkerSDK/improbable/c_worker.h>

#include "DeploymentsPlayerController.generated.h"

USTRUCT(BlueprintType)
struct FDeploymentInfo {
	GENERATED_BODY()

		UPROPERTY(BlueprintReadOnly)
		FString DeploymentId;
	UPROPERTY(BlueprintReadOnly)
		FString DeploymentName;
	UPROPERTY(BlueprintReadOnly)
		FString LoginToken;
	UPROPERTY(BlueprintReadOnly)
		int32 PlayerCount = 0;
	UPROPERTY(BlueprintReadOnly)
		int32 MaxPlayerCount = 0;
	UPROPERTY(BlueprintReadOnly)
		bool bAvailable = false;
};

UCLASS()
class GDKSHOOTER_API ADeploymentsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeploymentsEvent, const TArray<FDeploymentInfo>&, DeploymentList);
	UPROPERTY(BlueprintAssignable)
		FDeploymentsEvent OnDeploymentsReceived;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoadingEvent);
	UPROPERTY(BlueprintAssignable)
		FLoadingEvent OnLoadingStarted;
	UPROPERTY(BlueprintAssignable)
		FLoadingEvent OnLoadingFailed;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	void Populate(const Worker_Alpha_LoginTokensResponse* Deployments);
	FString LatestPIToken;
	const char * LatestPITokenData;

	FTimerHandle QueryDeploymentsTimer;

	UFUNCTION(BlueprintCallable)
		void JoinDeployment(const FString& LoginToken);

	UFUNCTION(BlueprintCallable)
		void SetLoadingScreen(UUserWidget* LoadingScreen);
	
private:
	void ScheduleRefreshDeployments();
};
