 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPawn.h"

float ACrossServerPawn::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	TakeDamageCrossServer(Damage, DamageEvent, nullptr, DamageCauser);
	return Damage;
}

void ACrossServerPawn::TakeDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	IncomingDamage.Broadcast(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
}
