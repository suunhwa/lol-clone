#include "Characters/Object/LoLInhibitor.h"
#include "Components/ObjectStatComponent.h"

ALoLInhibitor::ALoLInhibitor() { ObjectID = 11201; }

void ALoLInhibitor::OnDestroyed()
{
	Super::OnDestroyed();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ALoLInhibitor::Respawn, MechData.Respawn_Time, false);
}

void ALoLInhibitor::Respawn()
{
	bIsDestroyed = false;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	ObjectStatComp->InitObjectStats(StatData, RewardData, MechData);
}