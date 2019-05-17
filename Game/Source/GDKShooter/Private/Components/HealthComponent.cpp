// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "HealthComponent.h"
#include "GDKLogging.h"
#include "UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	bReplicates = true;

	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	MaxArmour = 100;
	CurrentArmour = 0;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		//Timers if required for regen
		//TODO watch out for multiworker here
		CurrentHealth = MaxHealth;
		CurrentArmour = 0;
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);

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
	int32 ArmourRemoved = FMath::Min(static_cast<int32>(Damage), CurrentArmour);
	CurrentArmour -= ArmourRemoved;
	int32 DamageDealt = FMath::Min(static_cast<int32>(Damage) - ArmourRemoved, CurrentHealth);
	bool bWasDead = CurrentHealth <= 0;
	CurrentHealth -= DamageDealt;
	bool bIsDead = CurrentHealth <= 0;

	MulticastDamageTaken(DamageCauser->GetActorLocation());

	if (!bWasDead && bIsDead)
	{
		//This will only fire on theauthoritative worker
		AuthoritativeDeath.Broadcast(DamageCauser);
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
		CurrentHealth = FMath::Min(static_cast<int32>(CurrentHealth + Value), MaxHealth);

		return true;
	}

	return false;
}

bool UHealthComponent::GrantShield(float Value)
{
	if (CurrentArmour < MaxArmour)
	{
		CurrentArmour = FMath::Min(static_cast<int32>(CurrentArmour + Value), MaxArmour);

		return true;
	}

	return false;
}

void UHealthComponent::RegenerateHealth()
{
	if (CurrentHealth > 0)
	{
		GrantHealth(HealthRegenValue);
	}
}

void UHealthComponent::RegenerateArmour()
{
	if (CurrentArmour > 0)
	{
		GrantHealth(ArmourRegenValue);
	}
}

void UHealthComponent::OnRep_CurrentArmour()
{
	ArmourUpdated.Broadcast(CurrentArmour, MaxArmour);
}

void UHealthComponent::OnRep_CurrentHealth()
{
	HealthUpdated.Broadcast(CurrentHealth, MaxHealth);
	if (CurrentHealth == 0)
	{
		Death.Broadcast();
	}
}

void UHealthComponent::MulticastDamageTaken_Implementation(FVector DamageSource)
{
	DamageTaken.Broadcast(DamageSource);
}
