// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EquippedComponent.h"
#include "UnrealNetwork.h"
#include "GDKLogging.h"
#include "Weapons/Holdable.h"


UEquippedComponent::UEquippedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bReplicates = true;
}


void UEquippedComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (CurrentlyHeldItem())
	{
		CurrentlyHeldItem()->SetIsActive(true);
	}
	
}

void UEquippedComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetNetMode() == NM_DedicatedServer && GetOwner()->HasAuthority())
	{
		for (AHoldable* Holdable : HeldItems)
		{
			if (Holdable != nullptr && !Holdable->IsPendingKill())
			{
				GetWorld()->DestroyActor(Holdable);
			}
		}
	}
}

void UEquippedComponent::SpawnStarterTemplates(FGDKMetaData MetaData)
{
	if (!bHeldItemsInitialised && GetOwner()->HasAuthority())
	{
		HeldItems.SetNum(HoldableCapacity, false);
		bHeldItemsInitialised = true;

		for (int i = 0; i < FMath::Min(StarterTemplates.Num(), HoldableCapacity); i++)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			AHoldable* Starter = GetWorld()->SpawnActor<AHoldable>(StarterTemplates[i], GetOwner()->GetActorTransform(), SpawnParams);
			Starter->AssignTo(this);
			Starter->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Starter->SetMetaData(MetaData);
			Starter->SetOwner(GetOwner());
			HeldItems[i] = Starter;
		}

		if (StarterTemplates.Num() > 0)
		{
			CurrentHeldIndex = 0;
		}

		if (CurrentlyHeldItem())
		{
			CurrentlyHeldItem()->SetIsActive(true);
		}
	}
}

void UEquippedComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquippedComponent, CurrentHeldIndex);
	DOREPLIFETIME(UEquippedComponent, HeldItems);
	DOREPLIFETIME(UEquippedComponent, bHeldItemsInitialised);
}

void UEquippedComponent::OnRep_HeldUpdate()
{
	if (CurrentlyHeldItem())
	{
		LocallyActivate(CurrentlyHeldItem());
	}
}

AHoldable* UEquippedComponent::CurrentlyHeldItem() const
{
	if (CurrentHeldIndex < 0 || CurrentHeldIndex >= HeldItems.Num())
		return nullptr;

	return HeldItems[CurrentHeldIndex];
}

// This method is called by the Holdable when it wakes up
// It informs the component that it is ready to be activated/attached
void UEquippedComponent::InformWielderOfWielded(AHoldable* WieldedHoldable)
{
	bool bIsActiveWeapon = (WieldedHoldable == CurrentlyHeldItem());

	if (bIsActiveWeapon)
	{
		LocallyActivate(WieldedHoldable);
	}
	else
	{
		WieldedHoldable->SetIsActive(false);
	}
}

void UEquippedComponent::LocallyActivate(AHoldable* Holdable)
{
	if (LocallyActiveHoldable)
	{
		LocallyActiveHoldable->SetIsActive(false);
	}

	SetBusy(false);
	Holdable->SetIsActive(true);
	LocallyActiveHoldable = Holdable;
	HoldableUpdated.Broadcast(Holdable);
}

void UEquippedComponent::ServerRequestEquip_Implementation(int TargetIndex)
{
	CurrentHeldIndex = TargetIndex;
}

bool UEquippedComponent::ServerRequestEquip_Validate(int TargetIndex)
{
	return true;
}

void UEquippedComponent::ForceCooldown(float Cooldown)
{
	if (CurrentlyHeldItem())
	{
		CurrentlyHeldItem()->ForceCooldown(Cooldown);
	}
}

void UEquippedComponent::BlockUsing(bool bBlock)
{
	bBlockUsing = bBlock;
	if (bBlock)
	{
		StopPrimaryUse();
		StopSecondaryUse();
	}
}

void UEquippedComponent::SetBusy(bool bNewBusy)
{
	bIsBusy = bNewBusy;
	BusyUpdated.Broadcast(bIsBusy);
}

void UEquippedComponent::StartPrimaryUse()
{
	if (bBlockUsing)
	{
		return;
	}

	if (CurrentlyHeldItem())
	{
		if (bIsSprinting)
		{
			CurrentlyHeldItem()->ForceCooldown(SprintRecoveryTime);
		}

		CurrentlyHeldItem()->StartPrimaryUse();
	}
}

void UEquippedComponent::StopPrimaryUse()
{
	if (CurrentlyHeldItem())
	{
		CurrentlyHeldItem()->StopPrimaryUse();
	}
}

void UEquippedComponent::StartSecondaryUse()
{
	if (bBlockUsing)
	{
		return;
	}

	if (CurrentlyHeldItem())
	{
		CurrentlyHeldItem()->StartSecondaryUse();
	}
}

void UEquippedComponent::StopSecondaryUse()
{
	if (CurrentlyHeldItem())
	{
		CurrentlyHeldItem()->StopSecondaryUse();
	}
}
