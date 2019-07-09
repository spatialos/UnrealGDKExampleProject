// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerState.h"
#include "PlayerPublisher.generated.h"

UENUM(BlueprintType)
enum class EPlayerProgress : uint8
{
	Connected		UMETA(DisplayName = "Connected"),
	InGame			UMETA(DisplayName = "InGame"),
	Finished		UMETA(DisplayName = "Finished"),
	Disconnected	UMETA(DisplayName = "Disconnected"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerEvent, APlayerState*, PlayerState, EPlayerProgress, Progress);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UPlayerPublisher : public UActorComponent
{
	GENERATED_BODY()

public:
	void PublishPlayer(APlayerState* PlayerState, EPlayerProgress Progress) { PlayerEvent.Broadcast(PlayerState, Progress); }

	UPROPERTY(BlueprintAssignable)
		FPlayerEvent PlayerEvent;
	
};
