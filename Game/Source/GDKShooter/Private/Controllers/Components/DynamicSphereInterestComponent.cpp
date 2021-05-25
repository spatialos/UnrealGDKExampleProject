// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "Controllers/Components/DynamicSphereInterestComponent.h"
#include "Schema/StandardLibrary.h"
#include "Interop/SpatialInterestConstraints.h"

void UDynamicSphereInterestComponent::BeginPlay()
{
	Super::BeginPlay();
	RootConstraint.SphereConstraint = SpatialGDK::SphereConstraint{};
}

void UDynamicSphereInterestComponent::UpdateQuery(const FVector NewCenter, const float NewRadius)
{
	const AActor* OwningActor = GetOwner();
	check(OwningActor != nullptr && OwningActor->HasAuthority());

	if (!NewCenter.Equals(SphereCenter) || !FMath::IsNearlyEqual(NewRadius, SphereRadius))
	{
		SphereCenter = NewCenter;
		SphereRadius = NewRadius;
		RootConstraint.SphereConstraint->Center = SpatialGDK::Coordinates::FromFVector(SphereCenter);
		RootConstraint.SphereConstraint->Radius = SphereRadius;

		NotifyChannelUpdateRequired();
	}
}

void UDynamicSphereInterestComponent::PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager, SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const
{
	TArray<SpatialGDK::QueryConstraint>& Constraints = OutFrequencyToQueryConstraints.FindOrAdd(RuntimeUpdateFrequency);
	Constraints.Push(RootConstraint);
}
