#pragma once
#include "AutomationCommon.h"
#include "Controllers/GDKPlayerController.h"
#include "Interop/SpatialStaticComponentView.h"
#include "SpatialAutomationCommon.h"
#include "SpatialNetDriver.h"

#if WITH_DEV_AUTOMATION_TESTS

SPATIAL_TEST(PlayerControllerGetsCreated, Launch)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(TestIfAtLeastGivenNumberOfActorsExist(AGDKPlayerController::StaticClass(), 1, this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

LATENT_TEST_COMMAND_ONE_PARAMETER(FGSMIsSetUpCorrectly, Test)
{
	UWorld* World = SpatialAutomationCommon::GetActiveGameWorld();
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());

	// All entities have position component on them, so getting false would mean that GSM entity itself doesn't exist.
	bool bGSMHasBeenCheckedOut = SpatialNetDriver->StaticComponentView->HasComponent(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID, SpatialConstants::POSITION_COMPONENT_ID);
	Test->TestTrue("GSM has been checked out by the server worker.", bGSMHasBeenCheckedOut);

	bool bAuthoritativeOverGSM = SpatialNetDriver->StaticComponentView->HasAuthority(SpatialConstants::INITIAL_GLOBAL_STATE_MANAGER_ENTITY_ID, SpatialConstants::POSITION_COMPONENT_ID);
	Test->TestTrue("Server has authority over GSM.", bAuthoritativeOverGSM);

	return true;
}

SPATIAL_TEST(GSMIsSetUpCorrectly, Launch)
{
	SpatialAutomationCommon::StartPIE();

	ADD_LATENT_AUTOMATION_COMMAND(FGSMIsSetUpCorrectly(this));

	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
