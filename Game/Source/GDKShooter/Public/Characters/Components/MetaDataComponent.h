// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MetaDataComponent.generated.h"


// Meta Data sctruct holding information about a player, also passed to a player's inventory items
USTRUCT(BlueprintType)
struct FGDKMetaData {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
		int32 Customization;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMetaDataUpdated, FGDKMetaData, MetaData);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GDKSHOOTER_API UMetaDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMetaDataComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE FGDKMetaData GetMetaData() const
	{
		return MetaData;
	}

	FORCEINLINE bool IsMetaDataAvailable() const
	{
		return bMetaDataAvailable;
	}

	UFUNCTION(BlueprintCallable)
		void SetMetaData(FGDKMetaData NewMetaData);

	UPROPERTY(BlueprintAssignable)
		FMetaDataUpdated MetaDataUpdated;

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MetaData)
		FGDKMetaData MetaData;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Replicated)
		bool bMetaDataAvailable = false;
	
	UFUNCTION()
		void OnRep_MetaData();
};
