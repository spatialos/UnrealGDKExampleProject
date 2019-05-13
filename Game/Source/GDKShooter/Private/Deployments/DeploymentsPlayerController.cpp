// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DeploymentsPlayerController.h"

#include "SpatialGameInstance.h"
#include "TimerManager.h"
#include "SpatialWorkerConnection.h"

#include "GDKLogging.h"


void ADeploymentsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	QueryPIT();
}

void ADeploymentsPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void OnLoginTokens(void* UserData, const Worker_Alpha_LoginTokensResponse* LoginTokens)
{
	ADeploymentsPlayerController* contoller = static_cast<ADeploymentsPlayerController*>(UserData);
	if (LoginTokens->status.code == WORKER_CONNECTION_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogGDK, Log, TEXT("Success: Login Token Count %d"), LoginTokens->login_token_count);
		contoller->Populate(LoginTokens);
	}
	else
	{
		UE_LOG(LogGDK, Log, TEXT("Failure: Error %s"), UTF8_TO_TCHAR(LoginTokens->status.detail));
	}
}

void OnPlayerIdentityToken(void* UserData, const Worker_Alpha_PlayerIdentityTokenResponse* PIToken)
{
	if (PIToken->status.code == WORKER_CONNECTION_STATUS_CODE_SUCCESS)
	{
		UE_LOG(LogGDK, Log, TEXT("Success: Received PIToken: %s"), UTF8_TO_TCHAR(PIToken->player_identity_token));
		ADeploymentsPlayerController* controller = static_cast<ADeploymentsPlayerController*>(UserData);
		controller->LatestPITokenData = PIToken->player_identity_token;
		controller->LatestPIToken = UTF8_TO_TCHAR(PIToken->player_identity_token);

		if (!controller->GetWorld()->GetTimerManager().IsTimerActive(controller->QueryDeploymentsTimer))
		{
			controller->GetWorld()->GetTimerManager().SetTimer(controller->QueryDeploymentsTimer, controller, &ADeploymentsPlayerController::QueryDeployments, 5.0f, true, 0.0f);
		}
	}
	else
	{
		UE_LOG(LogGDK, Log, TEXT("Failure: Error %s"), UTF8_TO_TCHAR(PIToken->status.detail));
		ADeploymentsPlayerController* controller = static_cast<ADeploymentsPlayerController*>(UserData);

		if (controller->GetWorld()->GetTimerManager().IsTimerActive(controller->QueryDeploymentsTimer))
		{
			controller->GetWorld()->GetTimerManager().ClearTimer(controller->QueryDeploymentsTimer);
		}
	}
}

void ADeploymentsPlayerController::QueryDeployments()
{
	Worker_Alpha_LoginTokensRequest* LTParams = new Worker_Alpha_LoginTokensRequest();
	LTParams->player_identity_token = LatestPITokenData;
	LTParams->worker_type = "UnrealClient";
	Worker_Alpha_LoginTokensResponseFuture* LTFuture = Worker_Alpha_CreateDevelopmentLoginTokensAsync("locator.improbable.io", 444, LTParams);
	Worker_Alpha_LoginTokensResponseFuture_Get(LTFuture, nullptr, this, OnLoginTokens);
}

void ADeploymentsPlayerController::QueryPIT()
{
	Worker_Alpha_PlayerIdentityTokenRequest* PITParams = new Worker_Alpha_PlayerIdentityTokenRequest();
	// Replace this string with a dev auth token, see docs for information on how to generate one of these
	PITParams->development_authentication_token_id = "ODU5OTg5MmEtY2YzZS00ZWMwLTk5ZTctNTNlNGM1M2QzZDA1OjoxMGNhMWFmMC05MjA2LTQwMTAtYWYxOC0wMzkyODdkMzI4ZDM=";
	PITParams->player_id = "Player Id";
	PITParams->display_name = "";
	PITParams->metadata = "";
	PITParams->use_insecure_connection = false;
	
	Worker_Alpha_PlayerIdentityTokenResponseFuture* PITFuture = Worker_Alpha_CreateDevelopmentPlayerIdentityTokenAsync("locator.improbable.io", 444, PITParams);

	if (PITFuture != nullptr)
	{
		Worker_Alpha_PlayerIdentityTokenResponseFuture_Get(PITFuture, nullptr, this, OnPlayerIdentityToken);
	}
}

FDeploymentInfo Parse(const Worker_Alpha_LoginTokenDetails LoginToken)
{
	FDeploymentInfo DeploymentInfo;

	DeploymentInfo.DeploymentId = UTF8_TO_TCHAR(LoginToken.deployment_id);
	DeploymentInfo.DeploymentName = UTF8_TO_TCHAR(LoginToken.deployment_name);
	DeploymentInfo.LoginToken = UTF8_TO_TCHAR(LoginToken.login_token);

	for (int i = 0; i < (int)LoginToken.tag_count; i++)
	{
		FString tag = UTF8_TO_TCHAR(LoginToken.tags[i]);
		if (tag.StartsWith("max_players_"))
		{
			tag.RemoveFromStart("max_players_");
			DeploymentInfo.MaxPlayerCount = FCString::Atoi(*tag);
		}
		else if (tag.StartsWith("players_"))
		{
			tag.RemoveFromStart("players_");
			DeploymentInfo.PlayerCount = FCString::Atoi(*tag);
		} else if (tag.Equals("status_lobby"))
		{
			DeploymentInfo.bAvailable = true;
		}
	}
	return DeploymentInfo;
}

void ADeploymentsPlayerController::Populate(const Worker_Alpha_LoginTokensResponse* Deployments)
{
	TArray<FDeploymentInfo> DeploymentArray;
	for (int i = 0; i < (int)Deployments->login_token_count; i++)
	{
		DeploymentArray.Add(Parse(Deployments->login_tokens[i]));
	}


	DeploymentArray.Sort([](const FDeploymentInfo& lhs, const FDeploymentInfo& rhs)
	{
		return lhs.DeploymentName.Compare(rhs.DeploymentName) < 0;
	});

	OnDeploymentsReceived.Broadcast(DeploymentArray);
}

void ADeploymentsPlayerController::JoinDeployment(const FString& LoginToken)
{
	FURL TravelURL;
	TravelURL.Host = TEXT("locator.improbable.io");
	TravelURL.AddOption(TEXT("locator"));
	TravelURL.AddOption(*FString::Printf(TEXT("playeridentity=%s"), *LatestPIToken));
	TravelURL.AddOption(*FString::Printf(TEXT("login=%s"), *LoginToken));
	
	OnLoadingStarted.Broadcast();

	ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false);
}

void ADeploymentsPlayerController::SetLoadingScreen(UUserWidget* LoadingScreen)
{
	GetGameInstance()->GetGameViewportClient()->AddViewportWidgetContent(LoadingScreen->TakeWidget());
}
