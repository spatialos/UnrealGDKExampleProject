// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "TestingObject.h"

#include "TestLevelScriptActor.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GDKSHOOTER_API ATestLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ATestingObject* TestObjectReference;
	
};
