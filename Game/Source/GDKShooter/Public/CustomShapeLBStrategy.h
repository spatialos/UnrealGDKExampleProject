// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "CoreMinimal.h"
#include "Math/Box2D.h"
#include "Math/Vector2D.h"

#include "CustomShapeLBStrategy.generated.h"

class SpatialVirtualWorkerTranslator;
class UAbstractSpatialMultiWorkerSettings;

DECLARE_LOG_CATEGORY_EXTERN(LogCustomShapeLBStrategy, Log, All)

/**
 * A load balancing strategy that divides the world into a grid.
 * Divides the load between Rows * Cols number of workers, each handling a
 * square of the world (WorldWidth / Cols)cm by (WorldHeight / Rows)cm
 *
 * Given a Point, for each Cell:
 * Point is inside Cell iff Min(Cell) <= Point < Max(Cell)
 *
 * Intended Usage: Create a data-only blueprint subclass and change
 * the Cols, Rows, WorldWidth, WorldHeight.
 */
UCLASS(Blueprintable, HideDropdown)
class GDKSHOOTER_API UCustomShapeLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	UCustomShapeLBStrategy();

	using LBStrategyRegions = TArray<TPair<VirtualWorkerId, FBox2D>>;

	/* UAbstractLBStrategy Interface */
	virtual void Init() override;

	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;
	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual bool RequiresHandoverData() const override { return WorkerCount > 1; }

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint32 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;
	/* End UAbstractLBStrategy Interface */

protected:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
		uint32 WorkerCount;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
		uint32 GridRows;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
		uint32 GridCols;

	UPROPERTY(EditDefaultsOnly, Category = "Grid Based Load Balancing")
		TArray<FString> WorkerGridIndices;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
		float WorldWidth;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"), Category = "Grid Based Load Balancing")
		float WorldHeight;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0"), Category = "Grid Based Load Balancing")
		float InterestBorder;

private:
	TArray<VirtualWorkerId> VirtualWorkerIds;

	TArray<TArray<FBox2D>> WorkerCellsSet;
	uint32 LocalCellId;
	bool bIsStrategyUsedOnLocalWorker;
	mutable bool bHasUpdatedSpatialDebugger;

	static bool IsInside(const FBox2D& Box, const FVector2D& Location);
	TMap<int, int> GetCellsToWorkerMap(const FString input);

	void UpdateSpatialDebugger() const;
};
