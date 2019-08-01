// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "ActorFunctionLibrary.h"
#include "GameFramework/Actor.h"

void UActorFunctionLibrary::RenameDisplayName(AActor* ToRename, const FString NewName)
{
	if (ToRename == nullptr)
	{
		return;
	}
#if WITH_EDITOR
	ToRename->SetActorLabel(*NewName);
#endif
}
