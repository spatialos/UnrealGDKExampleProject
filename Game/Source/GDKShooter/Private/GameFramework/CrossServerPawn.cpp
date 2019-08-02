 // Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPawn.h"

float ACrossServerPawn::TakeDamage(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	FVector Impact;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent* const PointDamageEvent = (FPointDamageEvent*)&DamageEvent;
		Impact = PointDamageEvent->HitInfo.ImpactPoint;
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		Impact = (DamageCauser->GetActorLocation() + GetActorLocation()) / 2.f;
	}
	else
	{
		Impact = GetActorLocation();
	}
	TakeDamageCrossServer(Damage, DamageEvent, nullptr, DamageCauser, Impact);
	return Damage;
}

void ACrossServerPawn::TakeDamageCrossServer_Implementation(float Damage, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser, FVector Location)
{
	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	IncomingDamage.Broadcast(ActualDamage, DamageEvent, EventInstigator, DamageCauser, Location);
}
