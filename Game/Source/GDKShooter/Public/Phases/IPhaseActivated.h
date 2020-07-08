#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IPhaseActivated.generated.h"

UINTERFACE(BlueprintType)
class GDKSHOOTER_API UPhaseActivated : public UInterface
{
	GENERATED_BODY()
};

class GDKSHOOTER_API IPhaseActivated
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Phases")
	void SnapToPhase(int32 Phase);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Phases")
	void ProgressToPhase(int32 Phase);
};
