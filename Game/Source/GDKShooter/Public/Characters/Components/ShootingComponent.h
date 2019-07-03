// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/ITraceProvider.h"
#include "ShootingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShotEvent, AWeapon*, Weapon);

USTRUCT(BlueprintType)
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

		// Location of the hit in world space.
		UPROPERTY(BlueprintReadOnly)
		FVector Location;

	// Actor that was hit, or nullptr if nothing was hit.
	UPROPERTY(BlueprintReadOnly)
		AActor* HitActor;

	UPROPERTY(BlueprintReadOnly)
		bool bDidHit;

	FInstantHitInfo() :
		Location(FVector{ 0,0,0 }),
		HitActor(nullptr),
		bDidHit(false)
	{}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UShootingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShootingComponent();

	void BeginPlay();

	UPROPERTY(BlueprintAssignable)
		FShotEvent ShotEvent;

	void FireShot(AWeapon* Weapon) { ShotEvent.Broadcast(Weapon); }

	UFUNCTION(BlueprintPure)
	FVector GetLineTraceStart()
	{
		UObject* TraceProviderObject = TraceProvider.GetObject();
		return ITraceProvider::Execute_GetLineTraceStart(TraceProvider.GetObject());
	}

	UFUNCTION(BlueprintPure)
	FVector GetLineTraceDirection()
	{
		UObject* TraceProviderObject = TraceProvider.GetObject();
		return ITraceProvider::Execute_GetLineTraceDirection(TraceProvider.GetObject());
	}

	UFUNCTION(BlueprintPure)
		FInstantHitInfo DoLineTrace(FVector Direction, AActor* ActorToIgnore = nullptr);
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		TScriptInterface<ITraceProvider> TraceProvider;

	// Maximum range of the weapon's hitscan.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shooting")
		float MaxRange;
	
	// Channel to use for raytrace on shot
	UPROPERTY(EditAnywhere, Category = "Shooting")
		TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

};
