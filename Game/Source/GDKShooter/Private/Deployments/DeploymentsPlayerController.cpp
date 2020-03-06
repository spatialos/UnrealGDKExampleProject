// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DeploymentsPlayerController.h"

#include "SpatialGameInstance.h"
#include "TimerManager.h"
#include "SpatialGDKSettings.h"
#include "SpatialWorkerConnection.h"

#include "GDKLogging.h"


void ADeploymentsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	USpatialGameInstance* SpatialGameInstance = GetGameInstance<USpatialGameInstance>();
	SpatialWorkerConnection = SpatialGameInstance->GetSpatialWorkerConnection();

	if (SpatialWorkerConnection == nullptr)
	{
		// We might not be using spatial networking in which case SpatialWorkerConnection will not exist so we should just return
		return;
	}

	FString SpatialWorkerType = SpatialGameInstance->GetSpatialWorkerType().ToString();
	SpatialWorkerConnection->RegisterOnLoginTokensCallback([this](const Worker_Alpha_LoginTokensResponse* Deployments){
		Populate(Deployments);
		if (!GetWorld()->GetTimerManager().IsTimerActive(QueryDeploymentsTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(QueryDeploymentsTimer, this,  &ADeploymentsPlayerController::ScheduleRefreshDeployments, 10.0f, true, 0.0f);
		}
		return true;
	});
	
	if (GetDefault<USpatialGDKSettings>()->bUseDevelopmentAuthenticationFlow)
	{
		SpatialWorkerConnection->Connect(true, 0);
	}
}

void ADeploymentsPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
	if (SpatialWorkerConnection != nullptr)
		SpatialWorkerConnection->RegisterOnLoginTokensCallback([](const Worker_Alpha_LoginTokensResponse* Deployments){return false;});
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
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
	if (SpatialWorkerConnection == nullptr)
	{
		UE_LOG(LogGDK, Error, TEXT("Failure: failed to Join Deployment caused by SpatialWorkerConnection is nullptr"));
		return;
	}

	const FLocatorConfig& LocatorConfig = SpatialWorkerConnection->LocatorConfig;
	FURL TravelURL;
	TravelURL.Host = LocatorConfig.LocatorHost;
	TravelURL.AddOption(TEXT("locator"));
	TravelURL.AddOption(*FString::Printf(TEXT("playeridentity=%s"), *LocatorConfig.PlayerIdentityToken));
	TravelURL.AddOption(*FString::Printf(TEXT("login=%s"), *LoginToken));

	OnLoadingStarted.Broadcast();

	ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false);
}

void ADeploymentsPlayerController::SetLoadingScreen(UUserWidget* LoadingScreen)
{
	GetGameInstance()->GetGameViewportClient()->AddViewportWidgetContent(LoadingScreen->TakeWidget());
}

void ADeploymentsPlayerController::ScheduleRefreshDeployments()
{
	if (SpatialWorkerConnection != nullptr)
		SpatialWorkerConnection->RequestDeploymentLoginTokens();
}
