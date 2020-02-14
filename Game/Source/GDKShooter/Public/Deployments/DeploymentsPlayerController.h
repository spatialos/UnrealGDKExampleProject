// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/Online/HTTP/Public/Http.h"
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

USTRUCT()
struct FRequest_Auth {
	GENERATED_BODY()
	UPROPERTY() FString PlayerID;

	FRequest_Auth() {}
};

USTRUCT()
struct FResponse_Auth {
	GENERATED_BODY()
	UPROPERTY() FString PlayerIdentityToken;

	FResponse_Auth() {}
};

USTRUCT()
struct FResponse_CreateTicket {
	GENERATED_BODY()
	UPROPERTY() FString TicketID;

	FResponse_CreateTicket() {}
};

USTRUCT()
struct FResponse_GetDeployment {
	GENERATED_BODY()
	UPROPERTY() FString DeploymentID;
	UPROPERTY() FString LoginToken;

	FResponse_GetDeployment() {}
};

UCLASS(Config=Game)
class GDKSHOOTER_API ADeploymentsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	FHttpModule* Http;

	UPROPERTY(Config)
	FString FakeAuthTarget;

	UPROPERTY(Config)
	FString FrontendTarget;

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

	void QueryDeployments();

	FTimerHandle QueryDeploymentsTimer;

	UFUNCTION(BlueprintCallable)
	void JoinDeployment(const FString& LoginToken);

	UFUNCTION(BlueprintCallable)
	void SetLoadingScreen(UUserWidget* LoadingScreen);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerIdentityTokenEvent, const FString&, PlayerIdentityToken);
	UPROPERTY(BlueprintAssignable)
	FPlayerIdentityTokenEvent OnPITCreationSucceeded;
	UPROPERTY(BlueprintAssignable)
	FPlayerIdentityTokenEvent OnPITCreationFailed;

	UFUNCTION(BlueprintCallable)
	void CreatePlayerIdentityToken(const FString& PlayerID);
	void OnPITResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOpenMatchTicketEvent, const FString&, TicketID);
	UPROPERTY(BlueprintAssignable)
	FOpenMatchTicketEvent OnOpenMatchTicketCreationSucceeded;
	UPROPERTY(BlueprintAssignable)
	FOpenMatchTicketEvent OnOpenMatchTicketCreationFailed;

	UFUNCTION(BlueprintCallable)
	void CreateOpenMatchTicket(const FString& PlayerIdentityToken);
	void OnOpenMatchTicketCreationResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOpenMatchDeploymentEvent, const FString&, DeploymentID, const FString&, LoginToken);
	UPROPERTY(BlueprintAssignable)
	FOpenMatchDeploymentEvent OnOpenMatchDeploymentMatched;
	UPROPERTY(BlueprintAssignable)
	FOpenMatchDeploymentEvent OnOpenMatchDeploymentMatchFailure;

	UFUNCTION(BlueprintCallable)
	void GetOpenMatchDeployment(const FString& PlayerIdentityToken, const FString& TicketID);
	void OnOpenMatchDeploymentResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

private:
	void QueryPIT();
	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);

	template<typename TStruct>
	FString StructToJsonString(TStruct& Input);
	
	template<typename TStruct>
	void JsonStringToStruct(FString Input, TStruct& Output);
};
