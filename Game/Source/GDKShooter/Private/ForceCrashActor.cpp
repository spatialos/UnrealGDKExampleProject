// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "ForceCrashActor.h"
#include "GDKLogging.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"

// Sets default values
AForceCrashActor::AForceCrashActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CrashTimer = 0.f;
	CrashLogTimer = 0.f;
}

// Called when the game starts or when spawned
void AForceCrashActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AForceCrashActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CrashTimer += DeltaTime;
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());

	if (NetDriver->LoadBalanceStrategy)
	{
		int vid = NetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
		if (CrashTimer - CrashLogTimer >= 1.0f)
		{
			UE_LOG(LogGDK, Error, TEXT("VID %d: Crash timer: %f"), vid, CrashTimer);
			CrashLogTimer = CrashTimer;
		}

		if (CrashTimer >= 120.f)
		{
			UE_LOG(LogGDK, Error, TEXT("VID %d: Crashing worker!"), vid);
			AActor* CrashPtr = nullptr;
			FString Name = CrashPtr->GetName();
		}
	}
}

