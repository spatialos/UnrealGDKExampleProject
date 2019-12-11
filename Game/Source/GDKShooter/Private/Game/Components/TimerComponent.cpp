// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Game/Components/TimerComponent.h"
#include "Engine/World.h"
#include "GDKLogging.h"
#include "Net/UnrealNetwork.h"

UTimerComponent::UTimerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
}

void UTimerComponent::BeginPlay()
{
	Super::BeginPlay();
	TimeLeft = DefaultTimerDuration;
	if (bAutoStart)
	{
		StartTimer();
	}
}

void UTimerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTimerComponent, bIsTimerRunning);
	DOREPLIFETIME(UTimerComponent, TimeLeft);
	DOREPLIFETIME(UTimerComponent, bHasTimerFinished);
}

void UTimerComponent::StartTimer()
{
	TimeLeft = DefaultTimerDuration;
	ResumeTimer();
}

void UTimerComponent::ResumeTimer()
{
	if (bIsTimerRunning)
	{
		return;
	}

	bIsTimerRunning = true;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UTimerComponent::DecrementTimer, 1.0f, true, 1.0f);
}

void UTimerComponent::SetTimer(int32 NewValue)
{
	TimeLeft = NewValue;
	bHasTimerFinished = false;
}

void UTimerComponent::StopTimer()
{
	bIsTimerRunning = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void UTimerComponent::OnRep_Timer()
{
	OnTimer.Broadcast(TimeLeft);
}

void UTimerComponent::OnRep_TimerFinished()
{
	if (bHasTimerFinished)
	{
		OnTimerFinished.Broadcast();
	}
}

void UTimerComponent::DecrementTimer()
{
	TimeLeft--;

	if (TimeLeft <= 0)
	{
		bHasTimerFinished = true;
		StopTimer();
		OnTimerFinished.Broadcast();
	}
}
