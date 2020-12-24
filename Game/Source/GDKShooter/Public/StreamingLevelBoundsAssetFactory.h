// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Factories/DataAssetFactory.h"
#include "StreamingLevelBoundsAssetFactory.generated.h"

/**
 * 
 */
UCLASS()
class GDKSHOOTER_API UStreamingLevelBoundsAssetFactory : public UDataAssetFactory
{
	GENERATED_UCLASS_BODY()

public:
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
