#pragma once

#include "AutomationEditorCommon.h"
#include "AutomationCommon.h"
#include "CoreMinimal.h"
#include "LogMacros.h"

#if WITH_DEV_AUTOMATION_TESTS

class SpawningCommon
{
public:

	template< class T >
	static AActor* SpawnActorAtOrigin();

};

#endif // WITH_DEV_AUTOMATION_TESTS
