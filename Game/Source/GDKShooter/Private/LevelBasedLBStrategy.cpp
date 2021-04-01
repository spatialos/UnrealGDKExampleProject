// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "LevelBasedLBStrategy.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"
#include "Engine/LevelBounds.h"
#include "Engine/WorldComposition.h"
#include "Math/Box.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogLevelBasedLBStrategy);

ULevelBasedLBStrategy::ULevelBasedLBStrategy()
	: InterestBorder(0)
{

}

void ULevelBasedLBStrategy::Init()
{
	if (StreamingLevelBoundsActor == nullptr)
	{
		StreamingLevelBoundsActor = StaticCast<AStreamingLevelBoundsActor*>(UGameplayStatics::GetActorOfClass(this, AStreamingLevelBoundsActor::StaticClass()));
	}
	check(StreamingLevelBoundsActor != nullptr);

	for (int32 i = 0; i < AuthorityRegions.Num(); i++)
	{
		for (FString PackageName : AuthorityRegions[i].PackageNames)
		{
			LevelNameToWorkerIdMapping.Add(PackageName, i + 1);
		}
	}
}

void ULevelBasedLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	UE_LOG(LogLevelBasedLBStrategy, Log, TEXT("Set LocalVirtualWorkerId from %d to %d"), LocalVirtualWorkerId, InLocalVirtualWorkerId);
	if (!VirtualWorkerIds.Contains(InLocalVirtualWorkerId))
	{
		// This worker is simulating an offloaded layer.
		bIsStrategyUsedOnLocalWorker = false;
	}
	else
	{
		bIsStrategyUsedOnLocalWorker = true;
	}
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
}

TSet<VirtualWorkerId> ULevelBasedLBStrategy::GetVirtualWorkerIds() const
{
	return TSet<VirtualWorkerId>(VirtualWorkerIds);
}

bool ULevelBasedLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogLevelBasedLBStrategy, Warning, TEXT("LevelBasedLBStrategy not ready to relinquish authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return false;
	}

	if (!bIsStrategyUsedOnLocalWorker)
	{
		UE_LOG(LogLevelBasedLBStrategy, Warning, TEXT("LevelBasedLBStrategy not used on local worker."));
		return false;
	}

	//UWorld* OwningWorld = GetWorld();
	//UPackage* LevelPackage = Actor.GetLevel()->GetOutermost();

	//FIntVector LevelPosition = FIntVector::ZeroValue;
	//if (LevelPackage->WorldTileInfo)
	//{
	//	LevelPosition = LevelPackage->WorldTileInfo->AbsolutePosition;
	//}

	//LevelPosition = LevelPosition - OwningWorld->OriginLocation;

	//FBox LevelBBox(ForceInit);
	//if (LevelPackage->WorldTileInfo)
	//{
	//	LevelBBox = LevelPackage->WorldTileInfo->Bounds.ShiftBy(FVector(LevelPosition));
	//}

	//FBox LevelBBox = Actor.GetLevel()->LevelBoundsActor->GetComponentsBoundingBox();

	//bool ShouldHaveAuthoriry = LevelBBox.IsInside(Actor.GetActorLocation());
	bool bShouldHaveAuthority = (WhoShouldHaveAuthority(Actor) == LocalVirtualWorkerId);
	if (!bShouldHaveAuthority)
	{
		UE_LOG(LogLevelBasedLBStrategy, Warning, TEXT("LevelBasedLBStrategy's local VirtualWorkerId %d != %d; Actor=%s; ActorLocation=%s"),
			LocalVirtualWorkerId, WhoShouldHaveAuthority(Actor), *AActor::GetDebugName(&Actor), *Actor.GetActorLocation().ToString());
	}

	return bShouldHaveAuthority;
}

VirtualWorkerId ULevelBasedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogLevelBasedLBStrategy, Warning, TEXT("LevelBasedLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	if (Actor.GetActorLocation().IsZero())
		return 1;

	FString ActorInPackageName;
	/*
	const TArray<ULevelStreaming*> StreamingLevels = Actor.GetWorld()->GetStreamingLevels();
	for (int32 TileIdx = 0; TileIdx < StreamingLevels.Num(); ++TileIdx)
	{
		ULevelStreaming* Tile = StreamingLevels[TileIdx];
		//FBox LevelBounds = Tile->GetStreamingVolumeBounds();
		FBox LevelBounds = Tile->GetLoadedLevel()->LevelBoundsActor->GetComponentsBoundingBox();
		if (LevelBounds.IsInsideOrOn(Actor.GetActorLocation()))
		{
			ActorInPackageName = Tile->GetWorldAssetPackageName();
			break;
		}
	}
	*/
	for (const TPair<FString, FBox>& Pair : StreamingLevelBoundsActor->LevelBounds)
	{
		if (Pair.Value.IsInside(Actor.GetActorLocation()))
		{
			ActorInPackageName = Pair.Key;
			break;
		}
	}

	if (!ActorInPackageName.IsEmpty())
	{
		/*
		for (int32 i = 0; i < AuthorityRegions.Num(); i++)
		{
			for (FString PackageName : AuthorityRegions[i].PackageNames)
			{
				if (ActorInPackageName.EndsWith(PackageName))
					return i + 1;
			}
		}
		*/
		return LevelNameToWorkerIdMapping[ActorInPackageName];
	}
	else
	{
		UE_LOG(LogLevelBasedLBStrategy, Error, TEXT("Actor %s is not in bounds of any WorldCompositionTile, position: %s."), *AActor::GetDebugName(&Actor), *Actor.GetActorLocation().ToString());
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	UE_LOG(LogLevelBasedLBStrategy, Error, TEXT("LevelBasedLBStrategy couldn't determine virtual worker for Actor %s at position %s"), *AActor::GetDebugName(&Actor), *Actor.GetActorLocation().ToString());
	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

SpatialGDK::QueryConstraint ULevelBasedLBStrategy::GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const
{
	check(IsReady());
	check(bIsStrategyUsedOnLocalWorker);

	SpatialGDK::QueryConstraint Constraint;
	const TArray<FString> PackagesNamesToCheck = AuthorityRegions[VirtualWorker - 1].PackageNames;
	/*
	const TArray<ULevelStreaming*> StreamingLevels = GetWorld()->GetStreamingLevels();
	for (int32 TileIdx = 0; TileIdx < StreamingLevels.Num(); ++TileIdx)
	{
		ULevelStreaming* Tile = StreamingLevels[TileIdx];
		FString AssetPackageName = Tile->GetWorldAssetPackageName();
		for (int32 NameIndex = 0; NameIndex < PackagesNamesToCheck.Num(); NameIndex++)
		{
			if (AssetPackageName.EndsWith(PackagesNamesToCheck[NameIndex]))
			{
				//FBox Bounds = Tile->GetStreamingVolumeBounds();
				FBox Bounds = Tile->GetLoadedLevel()->LevelBoundsActor->GetComponentsBoundingBox();
				Bounds.ExpandBy(InterestBorder);
				SpatialGDK::QueryConstraint OrConstraint;
				OrConstraint.BoxConstraint = SpatialGDK::BoxConstraint{ SpatialGDK::Coordinates::FromFVector(Bounds.GetCenter()), SpatialGDK::EdgeLength::FromFVector(Bounds.GetExtent()) };
				Constraint.OrConstraint.Add(OrConstraint);
			}
		}
	}
	*/
	for (FString PackageName : PackagesNamesToCheck)
	{
		SpatialGDK::QueryConstraint OrConstraint;
		FBox StreamingVolumeBounds = StreamingLevelBoundsActor->StreamingVolumeBounds[PackageName];
		OrConstraint.BoxConstraint = SpatialGDK::BoxConstraint
		{
			SpatialGDK::Coordinates::FromFVector(StreamingVolumeBounds.GetCenter()),
			SpatialGDK::EdgeLength::FromFVector(StreamingVolumeBounds.GetExtent())
		};
		Constraint.OrConstraint.Add(OrConstraint);
	}

	return Constraint;
}

FVector ULevelBasedLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	check(bIsStrategyUsedOnLocalWorker);
	FString PackageName = AuthorityRegions[LocalVirtualWorkerId - 1].PackageNames[0];
	/*
	const TArray<ULevelStreaming*> StreamingLevels = GetWorld()->GetStreamingLevels();
	for (int32 TileIdx = 0; TileIdx < StreamingLevels.Num(); ++TileIdx)
	{
		ULevelStreaming* Tile = StreamingLevels[TileIdx];
		if (Tile->GetWorldAssetPackageName().EndsWith(PackageName) && Tile->GetLoadedLevel())
		{
			TWeakObjectPtr<ALevelBounds> LevelBounds = Tile->GetLoadedLevel()->LevelBoundsActor;
			if (LevelBounds != nullptr)
			{
				return LevelBounds->GetComponentsBoundingBox().GetCenter();
			}
			return Tile->GetStreamingVolumeBounds().GetCenter();
		}
	}
	*/
	return StreamingLevelBoundsActor->LevelBounds[PackageName].GetCenter();
	//UE_LOG(LogLevelBasedLBStrategy, Error, TEXT("LevelBasedLBStrategy couldn't find a LevelStraming matches any package names: %s"), *FString::Join(AuthorityRegions[LocalVirtualWorkerId - 1].PackageNames, TEXT(",")));
	//return FVector::ZeroVector;
}

void ULevelBasedLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	UE_LOG(LogLevelBasedLBStrategy, Log, TEXT("Setting VirtualWorkerIds %d to %d"), FirstVirtualWorkerId, LastVirtualWorkerId);
	for (VirtualWorkerId CurrentVirtualWorkerId = FirstVirtualWorkerId; CurrentVirtualWorkerId <= LastVirtualWorkerId; CurrentVirtualWorkerId++)
	{
		VirtualWorkerIds.Add(CurrentVirtualWorkerId);
	}
}
