// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CustomShapeLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
//#include "SpatialDebugger.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"

#include "Templates/Tuple.h"

DEFINE_LOG_CATEGORY(LogCustomShapeLBStrategy);

UCustomShapeLBStrategy::UCustomShapeLBStrategy()
	: Super()
	, WorkerCount(1)
	, GridRows(1)
	, GridCols(1)
	, WorldWidth(1000000.f)
	, WorldHeight(1000000.f)
	, InterestBorder(0.f)
	, LocalCellId(0)
	, bIsStrategyUsedOnLocalWorker(false)
	, bHasUpdatedSpatialDebugger(false)
{
	for (uint32 i = 0; i < WorkerCount; i++)
	{
		WorkerGridIndices.Add(TEXT("1"));
	}
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

	// Split the map to rows by columns
	const float ColumnWidth = WorldWidth / GridCols;
	const float RowHeight = WorldHeight / GridRows;

	TMap<int, int> CellToWorkerMap = GetCellsToWorkerMap("CHANGE ME!");
	int areaIndex = 1;
	for (uint32 Col = 0; Col < GridCols; ++Col)
	{
		YMax = YMin + ColumnWidth;

		for (uint32 Row = 0; Row < GridRows; ++Row)
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

	UpdateSpatialDebugger();

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

SpatialGDK::QueryConstraint UCustomShapeLBStrategy::GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const
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
	return WorkerCount;
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
		&& Location.X < Box.Max.X && Location.Y < Box.Max.Y;
}

// TODO: Use the input instead of hardcoding the map!
TMap<int, int> UCustomShapeLBStrategy::GetCellsToWorkerMap(const FString input)
{
	// Initialise WorkerCells
	for (uint32 i = 0; i < WorkerCount; ++i)
	{
		TArray<FBox2D> WorkerCells;
		WorkerCellsSet.Add(WorkerCells);
	}

	// Initialise TMap with cell indices mapping to WorkerCells
	TMap<int, int> CellToWorkerMap;
	for (int32 i = 0; i < WorkerGridIndices.Num(); ++i)
	{
		TArray<FString> StrIndicies;
		WorkerGridIndices[i].ParseIntoArray(StrIndicies, TEXT(","), true);

		for (int32 j = 0; j < StrIndicies.Num(); ++j)
		{
			uint32 Index = FCString::Atoi(*StrIndicies[j]);
			CellToWorkerMap.Add(Index, i);
		}
	}

	return CellToWorkerMap;
}

void UCustomShapeLBStrategy::UpdateSpatialDebugger() const
{
	if (bHasUpdatedSpatialDebugger)
		return;

	const USpatialNetDriver* NetDriver = StaticCast<USpatialNetDriver*>(GetWorld()->GetNetDriver());
	if (NetDriver->SpatialDebugger == nullptr)
		return;

	NetDriver->SpatialDebugger->WorkerRegions.Empty();
	for (int WorkerIndex = 0; WorkerIndex < WorkerCellsSet.Num(); WorkerIndex++)
	{
		TArray<FBox2D> Cells = WorkerCellsSet[WorkerIndex];
		const FColor WorkerColor = FColor::MakeRandomColor();
		for (int CellIndex = 0; CellIndex < Cells.Num(); CellIndex++)
		{
			FWorkerRegionInfo WorkerRegionInfo;
			WorkerRegionInfo.Color = WorkerColor;
			WorkerRegionInfo.Extents = Cells[CellIndex];
			NetDriver->SpatialDebugger->WorkerRegions.Add(WorkerRegionInfo);
		}
	}
	bHasUpdatedSpatialDebugger = true;
}
