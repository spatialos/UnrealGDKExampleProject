// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "StreamingLevelBoundsAssetFactory.h"

UStreamingLevelBoundsAssetFactory::UStreamingLevelBoundsAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UStreamingLevelBoundsAsset::StaticClass();
	DataAssetClass = UStreamingLevelBoundsAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UStreamingLevelBoundsAssetFactory::ConfigureProperties()
{
	return true;
}

UObject* UStreamingLevelBoundsAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UStreamingLevelBoundsAsset>(InParent, Class, Name, Flags);
}

bool UStreamingLevelBoundsAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}
