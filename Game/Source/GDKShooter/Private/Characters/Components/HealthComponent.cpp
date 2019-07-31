// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "HealthComponent.h"
#include "Components/ControllerEventsComponent.h"
#include "Components/ScorePublisher.h"
#include "GameFramework/Pawn.h"
#include "TeamComponent.h"
#include "UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bReplicates = true;

	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
	MaxArmour = 100.f;
	CurrentArmour = 0.f;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
		CurrentArmour = 0.f;
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentArmour);
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetOwner()->GetWorldTimerManager().IsTimerActive(HealthRegenerationHandle))
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(HealthRegenerationHandle);
	}

	if (GetOwner()->GetWorldTimerManager().IsTimerActive(ArmourRegenerationHandle))
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(ArmourRegenerationHandle);
	}
}

void UHealthComponent::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (UTeamComponent* Team = Cast<UTeamComponent>(GetOwner()->GetComponentByClass(UTeamComponent::StaticClass())))
	{
		if (EventInstigator && !Team->CanDamageActor(EventInstigator->GetPawn()))
		{
			return;
		}
	}

	int32 ArmourRemoved = FMath::Min(Damage, CurrentArmour);
	CurrentArmour -= ArmourRemoved;
	int32 DamageDealt = FMath::Min(Damage - ArmourRemoved, CurrentHealth);
	bool bWasDead = CurrentHealth <= 0.f;
	CurrentHealth -= DamageDealt;
	bool bIsDead = CurrentHealth <= 0.f;

	int32 InstigatorPlayerId = -1;
	FGenericTeamId InstigatorTeamId = FGenericTeamId::NoTeam;
	if (EventInstigator != nullptr)
	{
		APlayerState* InstigatorPlayerState = EventInstigator->PlayerState;
		if (InstigatorPlayerState != nullptr)
		{
			InstigatorPlayerId = InstigatorPlayerState->PlayerId;
		}
	}
	if (IGenericTeamAgentInterface* InstgatorTeam = Cast<IGenericTeamAgentInterface>(EventInstigator))
	{
		InstigatorTeamId = InstgatorTeam->GetGenericTeamId();
	}
	else if (IGenericTeamAgentInterface* CauserTeam = Cast<IGenericTeamAgentInterface>(DamageCauser))
	{
		InstigatorTeamId = CauserTeam->GetGenericTeamId();
	}

	FVector Source;
	FVector Impact;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent* const PointDamageEvent = (FPointDamageEvent*)&DamageEvent;
		Source = DamageCauser ? DamageCauser->GetActorLocation() : GetOwner()->GetActorLocation() - PointDamageEvent->ShotDirection;
		Impact = PointDamageEvent->HitInfo.ImpactPoint;
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent* const RadialDamageEvent = (FRadialDamageEvent*)&DamageEvent;
		Impact = GetOwner()->GetActorLocation() + RadialDamageImpactOffset;
		Source = RadialDamageEvent->Origin;
	}
	else
	{
		Source = DamageCauser ? DamageCauser->GetActorLocation() : GetOwner()->GetActorLocation();
		Impact = GetOwner()->GetActorLocation();
	}

	MulticastDamageTaken(Damage, Source, Impact, InstigatorPlayerId, InstigatorTeamId);

	if (!bWasDead && bIsDead)
	{
		AuthoritativeDeath.Broadcast(EventInstigator);

		if (APawn* OwnerAsPawn = Cast<APawn>(GetOwner()))
		{
			if (AController* Controller = OwnerAsPawn->GetController())
			{
				if (UControllerEventsComponent* ControllerEvents = Cast<UControllerEventsComponent>(Controller->GetComponentByClass(UControllerEventsComponent::StaticClass())))
				{
					ControllerEvents->Death(EventInstigator);
				}

				if (EventInstigator != nullptr)
				{
					if (UControllerEventsComponent* ControllerEvents = Cast<UControllerEventsComponent>(EventInstigator->GetComponentByClass(UControllerEventsComponent::StaticClass())))
					{
						ControllerEvents->Kill(Controller);
					}
				}
			}
		}
	}


	if(!bIsDead)
	{
		if (GetOwner()->GetWorldTimerManager().IsTimerActive(HealthRegenerationHandle))
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(HealthRegenerationHandle);
		}
		if (GetOwner()->GetWorldTimerManager().IsTimerActive(ArmourRegenerationHandle))
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(ArmourRegenerationHandle);
		}
		if (HealthRegenInterval > 0)
		{
			GetOwner()->GetWorldTimerManager().SetTimer(HealthRegenerationHandle, this, &UHealthComponent::RegenerateHealth, HealthRegenInterval, true, HealthRegenCooldown);
		}
		if (ArmourRegenInterval > 0)
		{
			GetOwner()->GetWorldTimerManager().SetTimer(ArmourRegenerationHandle, this, &UHealthComponent::RegenerateArmour, ArmourRegenInterval, true, ArmourRegenCooldown);
		}
	}
}

bool UHealthComponent::GrantHealth(float Value)
{
	if (CurrentHealth < MaxHealth)
	{
		CurrentHealth = FMath::Min(CurrentHealth + Value, MaxHealth);

		return true;
	}

	return false;
}

bool UHealthComponent::GrantShield(float Value)
{
	if (CurrentArmour < MaxArmour)
	{
		CurrentArmour = FMath::Min(CurrentArmour + Value, MaxArmour);

		return true;
	}

	return false;
}

void UHealthComponent::RegenerateHealth()
{
	if (CurrentHealth > 0.f)
	{
		GrantHealth(HealthRegenValue);
	}
}

void UHealthComponent::RegenerateArmour()
{
	if (CurrentArmour > 0.f)
	{
		GrantHealth(ArmourRegenValue);
	}
}

void UHealthComponent::OnRep_CurrentArmour()
{
	ArmourUpdated.Broadcast(CurrentArmour, MaxArmour);
}


void UHealthComponent::MulticastDamageTaken_Implementation(float Value, FVector Source, FVector Impact, int32 PlayerId, FGenericTeamId TeamId)
{
	DamageTaken.Broadcast(Value, Source, Impact, PlayerId, TeamId);
}
