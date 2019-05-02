// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GDKMetaData.h"
#include "GDKPlayerState.generated.h"

UCLASS(SpatialType)
class GDKSHOOTER_API AGDKPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, Replicated)
	FGDKMetaData MetaData;
	
public:
	void SetMetaData(const FGDKMetaData& NewMetaData) { MetaData = NewMetaData; }
	const FGDKMetaData GetMetaData() const { return MetaData; }
};
