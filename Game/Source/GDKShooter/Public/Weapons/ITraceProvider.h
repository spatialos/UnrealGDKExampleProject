#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "ITraceProvider.generated.h"

UINTERFACE(BlueprintType)
class GDKSHOOTER_API UTraceProvider : public UInterface
{
	GENERATED_BODY()
};

class GDKSHOOTER_API ITraceProvider
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Trace Provider")
		FVector GetLineTraceStart() const;
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Trace Provider")
		FVector GetLineTraceDirection() const;
};
