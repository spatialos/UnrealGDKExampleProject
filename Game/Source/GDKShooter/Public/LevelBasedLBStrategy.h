// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "StreamingLevelBoundsActor.h"
#include "LevelBasedLBStrategy.generated.h"

class SpatialVirtualWorkerTranslator;
class UAbstractSpatialMultiWorkerSettings;

DECLARE_LOG_CATEGORY_EXTERN(LogLevelBasedLBStrategy, Log, All)

USTRUCT(BlueprintType)
struct FLevelRegion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FString> PackageNames;
};


UCLASS(Blueprintable)
class GDKSHOOTER_API ULevelBasedLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	ULevelBasedLBStrategy();

	virtual void Init() override;

	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;
	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual bool RequiresHandoverData() const override { return AuthorityRegions.Num() > 1; }

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint32 GetMinimumRequiredWorkers() const override { return AuthorityRegions.Num(); }
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;

protected:

	UPROPERTY(EditAnywhere)
	AStreamingLevelBoundsActor* StreamingLevelBoundsActor;

	UPROPERTY(EditDefaultsOnly)
	TArray<FLevelRegion> AuthorityRegions;

	UPROPERTY(EditDefaultsOnly)
	float InterestBorder;

private:
	bool bIsStrategyUsedOnLocalWorker;
	TArray<VirtualWorkerId> VirtualWorkerIds;
	TMap<FString, VirtualWorkerId> LevelNameToWorkerIdMapping;
};
