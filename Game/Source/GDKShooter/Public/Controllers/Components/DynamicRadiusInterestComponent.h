// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Schema/Interest.h"
#include "EngineClasses/Components/AbstractInterestComponent.h"
#include "DynamicRadiusInterestComponent.generated.h"

/**
 * 
 */
UCLASS(SpatialType = ServerOnly, Meta = (BlueprintSpawnableComponent))
class GDKSHOOTER_API UDynamicRadiusInterestComponent : public UAbstractInterestComponent
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = ""))
	FVector SphereCenter;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = ""))
	float SphereRadius;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Interest", meta = (ToolTip = "Frequency SpatialOS Runtime should run the query and send the result to the client (Hz)."))
	float RuntimeUpdateFrequency = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Interest")
	void UpdateQuery(const FVector NewCenter, const float NewRadius);

	// ISpatialInterestProvider
	virtual void PopulateFrequencyToConstraintsMap(const USpatialClassInfoManager& ClassInfoManager,
		SpatialGDK::FrequencyToConstraintsMap& OutFrequencyToQueryConstraints) const override;

private:
	SpatialGDK::QueryConstraint RootConstraint;
};
