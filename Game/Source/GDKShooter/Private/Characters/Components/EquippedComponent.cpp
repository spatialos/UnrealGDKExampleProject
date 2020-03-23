// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/Components/EquippedComponent.h"
#include "Net/UnrealNetwork.h"
#include "GDKLogging.h"
#include "Engine/World.h"
#include "Weapons/Holdable.h"


UEquippedComponent::UEquippedComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


void UEquippedComponent::BeginPlay()
{
	Super::BeginPlay();

	OnRep_HeldUpdate();
	
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

		for (int i = 0; i < StarterTemplates.Num(); i++)
		{
			if (StarterTemplates[i] == nullptr)
			{
				continue;
			}

			AHoldable* Starter = GetWorld()->SpawnActor<AHoldable>(StarterTemplates[i], GetOwner()->GetActorTransform());
			Starter->SetMetaData(MetaData);
			Grant(Starter);
		}

		// Default to holding the weapon in slot 0
		if (StarterTemplates.Num() > 0)
		{
			CurrentHeldIndex = 0;
		}

		OnRep_HeldUpdate();
	}
}

bool UEquippedComponent::Grant(AHoldable* NewHoldable)
{
	if (NewHoldable == nullptr || !HasAnyEmptySlots() || AlreadyHas(NewHoldable))
	{
		return false;
	}

	int Index = GetNextAvailableSlot();
	NewHoldable->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	NewHoldable->SetOwner(GetOwner());
	HeldItems[Index] = NewHoldable;
	CurrentHeldIndex = Index;
	OnRep_HeldUpdate();
	return true;
}

bool UEquippedComponent::HasAnyEmptySlots()
{
	for (int i = 0; i < HeldItems.Num(); i++)
	{
		if (HeldItems[i] == nullptr)
		{
			return true;
		}
	}
	return false;
}

int UEquippedComponent::GetNextAvailableSlot()
{
	for (int i = 0; i < HeldItems.Num(); i++)
	{
		if (HeldItems[i] == nullptr)
		{
			return i;
		}
	}
	return -1;
}

bool UEquippedComponent::AlreadyHas(AHoldable* NewHoldable)
{
	for (int i = 0; i < HeldItems.Num(); i++)
	{
		if (HeldItems[i] != nullptr && HeldItems[i]->IsA(NewHoldable->GetClass()))
		{
			return true;
		}
	}
	return false;
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
	for (int i = 0; i < HeldItems.Num(); i++)
	{
		if(!HeldItems[i])
		{
			continue;
		}

		if (i == CurrentHeldIndex)
		{
			LastCachedIndex = CurrentCachedIndex;
			CurrentCachedIndex = CurrentHeldIndex;
			LocallyActivate(CurrentlyHeldItem());
		}
		else
		{
			HeldItems[i]->SetIsActive(false);
		}
	}
}

AHoldable* UEquippedComponent::CurrentlyHeldItem() const
{
	if (CurrentHeldIndex < 0 || CurrentHeldIndex >= HeldItems.Num())
		return nullptr;

	return HeldItems[CurrentHeldIndex];
}

void UEquippedComponent::LocallyActivate(AHoldable* Holdable)
{
	if (LocallyActiveHoldable)
	{
		LocallyActiveHoldable->SetIsActive(false);
	}

	Holdable->SetIsActive(true);
	LocallyActiveHoldable = Holdable;
	HoldableUpdated.Broadcast(Holdable);
}

void UEquippedComponent::ToggleMode()
{
	if (CurrentlyHeldItem())
	{
		CurrentlyHeldItem()->ToggleMode();
	}
}

void UEquippedComponent::QuickToggle()
{
	ServerRequestEquip(LastCachedIndex);
}

bool UEquippedComponent::HasHoldableAtIndex(int32 Index)
{
	return Index >= 0 && Index < HeldItems.Num() && HeldItems[Index];
}

void UEquippedComponent::ScrollUp()
{
	for (int i = CurrentHeldIndex + 1; i < HeldItems.Num(); i++)
	{
		if (HasHoldableAtIndex(i))
		{
			ServerRequestEquip(i);
			return;
		}
	}
}

void UEquippedComponent::ScrollDown()
{
	for (int i = CurrentHeldIndex - 1; i >= 0; i--)
	{
		if (HasHoldableAtIndex(i))
		{
			ServerRequestEquip(i);
			return;
		}
	}
}

void UEquippedComponent::ServerRequestEquip_Implementation(int32 TargetIndex)
{
	if (HasHoldableAtIndex(TargetIndex))
	{
		CurrentHeldIndex = TargetIndex;
	}
	OnRep_HeldUpdate();
}

bool UEquippedComponent::ServerRequestEquip_Validate(int32 TargetIndex)
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
