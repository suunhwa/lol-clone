#include "Characters/Minion/LoLMinion_Melee.h"

ALoLMinion_Melee::ALoLMinion_Melee()
{
	MinionID = 3001;
	
	// 충돌체 필요하면 꺼내쓰자
	// GetCapsuleComponent()->SetCapsuleHalfHeight(88.f);
}

void ALoLMinion_Melee::ExecuteAttack()
{
	Super::ExecuteAttack();
}

void ALoLMinion_Melee::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Melee Minion (3001) Spawned and Stats Initialized via Subsystem."));
	}
}
