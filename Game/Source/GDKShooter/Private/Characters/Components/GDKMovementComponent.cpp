// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Characters/Components/GDKMovementComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "GDKLogging.h"

// Use the first custom movement flag slot in the character for sprinting.
static const FSavedMove_Character::CompressedFlags FLAG_WantsToSprint = FSavedMove_GDKMovement::FLAG_Custom_0;

UGDKMovementComponent::UGDKMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MaxJogSpeed(450)
	, bCanSprint(true)
	, bWantsToSprint(false)
	, bWasSprintingLastFrame(false)
	, bIsAiming(false)
	, bIsBusy(false)
	, bShouldOrientToControlRotation(false)
	, MaxSprintSpeed(850)
	, SprintAcceleration(3400)
	, SprintDirectionTolerance(0.1f)
	, JogAcceleration(1800)
{
	MaxWalkSpeed = 250;
	MaxWalkSpeedCrouched = 125;
	MaxAcceleration = 1000;
	JumpZVelocity = 600.f;
	AirControl = 0.2f;
	SetIsReplicatedByDefault(true);
	//this->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
}

void UGDKMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsSprinting() != bWasSprintingLastFrame) {
		bWasSprintingLastFrame = IsSprinting();
		SprintingUpdated.Broadcast(IsSprinting());
	}
}

void UGDKMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGDKMovementComponent, bIsAiming);
}

void UGDKMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// Extract saved state from Flags and apply it to our local variables.
	bWantsToSprint = (Flags & FLAG_WantsToSprint) != 0;
}

class FNetworkPredictionData_Client* UGDKMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);
	check(PawnOwner->Role < ROLE_Authority);

	if (!ClientPredictionData)
	{
		UGDKMovementComponent* MutableThis = const_cast<UGDKMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_GDKMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = NetworkMaxSmoothUpdateDistance;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = NetworkNoSmoothUpdateDistance;
	}

	return ClientPredictionData;
}

void UGDKMovementComponent::SetSprintEnabled(bool bSprintEnabled)
{
	bCanSprint = bSprintEnabled;
}

void UGDKMovementComponent::SetWantsToSprint(bool bSprinting)
{
	bWantsToSprint = bSprinting;
}

bool UGDKMovementComponent::IsSprinting() const
{
	return
		static_cast<bool>(bCanSprint)
		&& static_cast<bool>(bWantsToSprint)
		&& IsMovingForward()
		&& !IsCrouching()
		&& !IsAiming()
		&& !IsBusy();
}

bool UGDKMovementComponent::IsAiming() const
{
	return bIsAiming;
}

float UGDKMovementComponent::GetMaxSpeed() const
{
	if (IsCrouching())
	{
		return MaxWalkSpeedCrouched;
	}
	else if (IsAiming())
	{
		return MaxWalkSpeed;
	}
	else if (IsSprinting())
	{
		return MaxSprintSpeed;
	}
	return MaxJogSpeed;
}

float UGDKMovementComponent::GetMaxAcceleration() const
{
	if (IsCrouching())
	{
		return MaxAcceleration;
	}
	else if (IsAiming())
	{
		return MaxAcceleration;
	}
	else if (IsSprinting())
	{
		return SprintAcceleration;
	}
	return JogAcceleration;
}

bool UGDKMovementComponent::IsMovingForward() const
{
	if (PawnOwner == nullptr)
	{
		return false;
	}

	FVector MoveDirection = Velocity.GetSafeNormal();
	FVector Forward = PawnOwner->GetActorForwardVector();
	if (AController* PlayerController = PawnOwner->GetController())
	{
		// Check move direction against control rotation.
		Forward = PlayerController->GetControlRotation().Vector();
	}

	// Ignore the Z axis for comparison.
	Forward.Z = 0.0f;
	MoveDirection.Z = 0.0f;
	Forward.Normalize();
	MoveDirection.Normalize();

	float VelocityDot = FVector::DotProduct(Forward, MoveDirection);

	return VelocityDot > SprintDirectionTolerance;
}

void FSavedMove_GDKMovement::Clear()
{
	Super::Clear();
	bSavedWantsToSprint = false;
}

uint8 FSavedMove_GDKMovement::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedWantsToSprint)
	{
		Result |= FLAG_WantsToSprint;
	}
	return Result;
}

bool FSavedMove_GDKMovement::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InPawn, float MaxDelta) const
{
	if (bSavedWantsToSprint != ((FSavedMove_GDKMovement*)&NewMove)->bSavedWantsToSprint)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InPawn, MaxDelta);
}

void FSavedMove_GDKMovement::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UGDKMovementComponent* CharacterMovement = Cast<UGDKMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bSavedWantsToSprint = CharacterMovement->bWantsToSprint;
	}
}

FNetworkPredictionData_Client_GDKMovement::FNetworkPredictionData_Client_GDKMovement(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{ }

FSavedMovePtr FNetworkPredictionData_Client_GDKMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_GDKMovement());
}

bool UGDKMovementComponent::CanCrouchInCurrentState() const
{
	if (!CanEverCrouch())
	{
		return false;
	}

	return !IsFalling() && IsMovingOnGround() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UGDKMovementComponent::ServerSetAiming_Validate(bool NewValue)
{
	return true;
}

void UGDKMovementComponent::ServerSetAiming_Implementation(bool NewValue)
{
	bIsAiming = NewValue;
	OnServerAimingUpdated.Broadcast(bIsAiming);
}

void UGDKMovementComponent::SetAiming(bool NewValue)
{
	bIsAiming = NewValue;
	if (GetOwnerRole() != ROLE_SimulatedProxy)
	{
		ServerSetAiming(NewValue);
	}
	OnAimingUpdated.Broadcast(bIsAiming);
}

void UGDKMovementComponent::OnRep_IsAiming()
{
	OnAimingUpdated.Broadcast(bIsAiming);
}
