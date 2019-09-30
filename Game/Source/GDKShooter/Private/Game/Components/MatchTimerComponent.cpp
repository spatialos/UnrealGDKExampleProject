// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "MatchTimerComponent.h"
#include "Misc/CommandLine.h"

void UMatchTimerComponent::BeginPlay()
{
#if !UE_BUILD_SHIPPING
	// Developer cheat so you can run an arbitrarily long match for testing.
	const TCHAR* CommandLine = FCommandLine::Get();
	FParse::Value(CommandLine, TEXT("matchtime"), DefaultTimerDuration);
#endif
	Super::BeginPlay();
}
