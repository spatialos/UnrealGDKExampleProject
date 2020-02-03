// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DeploymentsPlayerController.h"

#include "SpatialGameInstance.h"
#include "TimerManager.h"
#include "SpatialWorkerConnection.h"

#include "GDKLogging.h"


void ADeploymentsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ActivateTouchInterface(nullptr);
	bShowMouseCursor = true;
    
	auto SpatialGameInstance = GetGameInstance<USpatialGameInstance>();
	auto SpatialWorkerConnection = SpatialGameInstance->GetSpatialWorkerConnection();
	if (!SpatialWorkerConnection) {
		UE_LOG(LogGDK, Log, TEXT("Failure: failed to get SpatialWorkerConnection"));
		return;
	}
	
	SpatialWorkerConnection->RegisterOnLoginTokensCallback(std::bind(&ADeploymentsPlayerController::Populate, this, std::placeholders::_1));
	SpatialWorkerConnection->SetConnectionType(ESpatialConnectionType::DevAuthFlow);
	SpatialWorkerConnection->DevAuthConfig.DevelopmentAuthToken = TEXT("OGFjOGEwY2UtOTc1Yy00ZDQzLWE5ZTItYzAzMGM2NDU4ZGEzOjozMjE1OWU4Yy1lYmY2LTQzM2ItYjJiZi1hMDM4MjhkYTg1ZWE=");
	SpatialWorkerConnection->Connect(true, 0);
}

void ADeploymentsPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
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

bool ADeploymentsPlayerController::Populate(const Worker_Alpha_LoginTokensResponse* Deployments)
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
    return true;
}

void ADeploymentsPlayerController::JoinDeployment(const FString& LoginToken)
{
    auto SpatialGameInstance = GetGameInstance<USpatialGameInstance>();
    auto SpatialWorkerConnection = SpatialGameInstance->GetSpatialWorkerConnection();
    if (!SpatialWorkerConnection) {
		UE_LOG(LogGDK, Log, TEXT("Failure: failed to get SpatialWorkerConnection"));
		return;
	}
    
	const auto& DevAuthConfig = SpatialWorkerConnection->DevAuthConfig;
	FURL TravelURL;
	TravelURL.Host = DevAuthConfig.LocatorHost;
	TravelURL.AddOption(TEXT("locator"));
	TravelURL.AddOption(*FString::Printf(TEXT("playeridentity=%s"), *DevAuthConfig.PlayerIdentityToken));
	TravelURL.AddOption(*FString::Printf(TEXT("login=%s"), *LoginToken));
	
	OnLoadingStarted.Broadcast();

	ClientTravel(TravelURL.ToString(), TRAVEL_Absolute, false);
}

void ADeploymentsPlayerController::SetLoadingScreen(UUserWidget* LoadingScreen)
{
	GetGameInstance()->GetGameViewportClient()->AddViewportWidgetContent(LoadingScreen->TakeWidget());
}
