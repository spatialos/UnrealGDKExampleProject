// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "CustomShapeLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"

#include "Templates/Tuple.h"

DEFINE_LOG_CATEGORY(LogCustomShapeLBStrategy);

UCustomShapeLBStrategy::UCustomShapeLBStrategy()
	: Super()
{
}

void UCustomShapeLBStrategy::Init()
{
	Super::Init();
	UE_LOG(LogCustomShapeLBStrategy, Log, TEXT("CustomShapeLBStrategy initialized with Rows = %d and Cols = %d."), 2, 2);

	const float WorldWidthMin = -(WorldWidth / 2.f);
	const float WorldHeightMin = -(WorldHeight / 2.f);

	// We would like the inspector's representation of the load balancing strategy to match our intuition.
	// +x is forward, so rows are perpendicular to the x-axis and columns are perpendicular to the y-axis.
	float XMin = WorldHeightMin;
	float YMin = WorldWidthMin;
	float XMax, YMax;

	// Split the map to 9 cells, 3 rows by 3 columns
	const float ColumnWidth = WorldWidth / 3;
	const float RowHeight = WorldHeight / 3;

	TMap<int, int> CellToWorkerMap = GetCellsToWorkerMap("CHANGE ME!");
	int areaIndex = 1;
	for (uint32 Col = 0; Col < 3; ++Col)
	{
		YMax = YMin + ColumnWidth;

		for (uint32 Row = 0; Row < 3; ++Row)
		{
			XMax = XMin + RowHeight;

			FVector2D Min(XMin, YMin);
			FVector2D Max(XMax, YMax);
			FBox2D Cell(Min, Max);

			WorkerCellsSet[CellToWorkerMap[areaIndex]].Add(Cell);
			UE_LOG(LogCustomShapeLBStrategy, Log, TEXT("CustomShapeLBStrategy init adding cells to worker cells for index %d."), areaIndex);

			XMin = XMax;
			areaIndex++;
		}

		XMin = WorldHeightMin;
		YMin = YMax;
	}
}

void UCustomShapeLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	if (!VirtualWorkerIds.Contains(InLocalVirtualWorkerId))
	{
		// This worker is simulating a layer which is not part of the grid.
		LocalCellId = WorkerCellsSet.Num();
		bIsStrategyUsedOnLocalWorker = false;
	}
	else
	{
		LocalCellId = VirtualWorkerIds.IndexOfByKey(InLocalVirtualWorkerId);
		bIsStrategyUsedOnLocalWorker = true;
	}
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
}

TSet<VirtualWorkerId> UCustomShapeLBStrategy::GetVirtualWorkerIds() const
{
	return TSet<VirtualWorkerId>(VirtualWorkerIds);
}

bool UCustomShapeLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogCustomShapeLBStrategy, Warning, TEXT("CustomShapeLBStrategy not ready to relinquish authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return false;
	}

	if (!bIsStrategyUsedOnLocalWorker)
	{
		return false;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));
	for (int i = 0; i < WorkerCellsSet[LocalCellId].Num(); ++i)
	{
		if (IsInside(WorkerCellsSet[LocalCellId][i], Actor2DLocation))
		{
			return true;
		}
	}
	return false;
}

VirtualWorkerId UCustomShapeLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogCustomShapeLBStrategy, Warning, TEXT("CustomShapeLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const FVector2D Actor2DLocation = FVector2D(SpatialGDK::GetActorSpatialPosition(&Actor));

	check(VirtualWorkerIds.Num() == WorkerCellsSet.Num());
	for (int i = 0; i < WorkerCellsSet.Num(); ++i)
	{
		for (int j = 0; j < WorkerCellsSet[i].Num(); ++j)
		{
			if (IsInside(WorkerCellsSet[i][j], Actor2DLocation))
			{
				UE_LOG(LogCustomShapeLBStrategy, Log, TEXT("Actor: %s, grid %d, worker %d for position %s"), *AActor::GetDebugName(&Actor), i, VirtualWorkerIds[i], *Actor2DLocation.ToString());
				return VirtualWorkerIds[i];
			}
		}
	}

	UE_LOG(LogCustomShapeLBStrategy, Error, TEXT("CustomShapeLBStrategy couldn't determine virtual worker for Actor %s at position %s"), *AActor::GetDebugName(&Actor), *Actor2DLocation.ToString());
	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

SpatialGDK::QueryConstraint UCustomShapeLBStrategy::GetWorkerInterestQueryConstraint() const
{
	// For a grid-based strategy, the interest area is the cell that the worker is authoritative over plus some border region.
	check(IsReady());
	check(bIsStrategyUsedOnLocalWorker);

	SpatialGDK::QueryConstraint Constraint;
	for (int i = 0; i < WorkerCellsSet[LocalCellId].Num(); ++i)
	{
		const FBox2D Interest2D = WorkerCellsSet[LocalCellId][i].ExpandBy(InterestBorder);

		const FVector2D Center2D = Interest2D.GetCenter();
		const FVector Center3D{ Center2D.X, Center2D.Y, 0.0f };

		const FVector2D EdgeLengths2D = Interest2D.GetSize();
		check(EdgeLengths2D.X > 0.0f && EdgeLengths2D.Y > 0.0f);
		const FVector EdgeLengths3D{ EdgeLengths2D.X, EdgeLengths2D.Y, FLT_MAX };
		SpatialGDK::QueryConstraint BoxConstraint;

		BoxConstraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Center3D), SpatialGDK::EdgeLength::FromFVector(EdgeLengths3D) };
		Constraint.OrConstraint.Add(BoxConstraint);
	}

	return Constraint;
}

FVector UCustomShapeLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	check(bIsStrategyUsedOnLocalWorker);
	UE_LOG(LogCustomShapeLBStrategy, Log, TEXT("CustomShapeLBStrategy get worker entity with local call id %d"), LocalCellId);
	const FVector2D Centre = WorkerCellsSet[LocalCellId][0].GetCenter();
	return FVector{ Centre.X, Centre.Y, 0.f };
}

uint32 UCustomShapeLBStrategy::GetMinimumRequiredWorkers() const
{
	return Rows * Cols;
}

void UCustomShapeLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	UE_LOG(LogCustomShapeLBStrategy, Log, TEXT("Setting VirtualWorkerIds %d to %d"), FirstVirtualWorkerId, LastVirtualWorkerId);
	for (VirtualWorkerId CurrentVirtualWorkerId = FirstVirtualWorkerId; CurrentVirtualWorkerId <= LastVirtualWorkerId; CurrentVirtualWorkerId++)
	{
		VirtualWorkerIds.Add(CurrentVirtualWorkerId);
	}
}

bool UCustomShapeLBStrategy::IsInside(const FBox2D& Box, const FVector2D& Location)
{
	return Location.X >= Box.Min.X && Location.Y >= Box.Min.Y
		&& Location.X < Box.Max.X&& Location.Y < Box.Max.Y;
}

// TODO: Use the input instead of hardcoding the map!
TMap<int, int> UCustomShapeLBStrategy::GetCellsToWorkerMap(const FString input)
{
	// Create a combination of 1, 2, 3; 4; 5, 6, 7, 8, 9; LB shape
	TArray<int> Cells1;
	Cells1.Add(1);
	Cells1.Add(2);
	Cells1.Add(3);
	Cells1.Add(4);
	Cells1.Add(5);
	Cells1.Add(6);
	Cells1.Add(7);

	TArray<int> Cells2;
	Cells2.Add(8);

	TArray<int> Cells3;
	Cells3.Add(9);

	TArray<TArray<int>> AllCells;
	AllCells.Add(Cells1);
	AllCells.Add(Cells2);
	AllCells.Add(Cells3);

	// Initialise WorkerCells
	for (int i = 0; i < 3; ++i)
	{
		TArray<FBox2D> TmpWorkerCells;
		WorkerCellsSet.Add(TmpWorkerCells);
	}

	// Initialise TMap with cell indices mapping to WorkerCells
	TMap<int, int> CellToWorkerMap;
	for (int i = 0; i < AllCells.Num(); ++i)
	{
		for (int j = 0; j < AllCells[i].Num(); ++j)
		{
			CellToWorkerMap.Add(AllCells[i][j], i);
		}
	}

	return CellToWorkerMap;
}

UGridBasedLBStrategy::LBStrategyRegions UCustomShapeLBStrategy::GetLBStrategyRegions() const
{
	LBStrategyRegions VirtualWorkerToCell;
	VirtualWorkerToCell.SetNum(9);

	int index = 0;

	for (int i = 0; i < WorkerCellsSet.Num(); i++)
	{
		for (int j = 0; j < WorkerCellsSet[i].Num(); j++)
		{
			VirtualWorkerToCell[index++] = MakeTuple(VirtualWorkerIds[i], WorkerCellsSet[i][j]);
		}
	}
	return VirtualWorkerToCell;
}

