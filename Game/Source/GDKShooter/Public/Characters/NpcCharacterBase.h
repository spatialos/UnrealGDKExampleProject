// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GDKCharacter.h"
#include "NpcCharacterBase.generated.h"

UCLASS()
class GDKSHOOTER_API ANpcCharacterBase : public AGDKCharacter
{
	GENERATED_BODY()

public:
	ANpcCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void OnAuthorityGained() override;
};
