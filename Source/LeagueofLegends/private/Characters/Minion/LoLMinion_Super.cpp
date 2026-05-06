#include "Characters/Minion/LoLMinion_Super.h"
#include "Components/MinionStatComponent.h"
#include "Components/CapsuleComponent.h"

ALoLMinion_Super::ALoLMinion_Super()
{
	MinionID = 3004;

	// 캡슐 컴포넌트의 크기 설정
	GetCapsuleComponent()->InitCapsuleSize(60.f, 100.f);
}

void ALoLMinion_Super::BeginPlay()
{
	Super::BeginPlay();
	
	if (UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp))
	{
		if (MinionStat->IsSuperMinion())
		{
			UE_LOG(LogTemp, Log, TEXT("Super Minion Stats Initialized: HP %.f, AD %.f"), 
				MinionStat->GetCurrentHP(), MinionStat->GetAD());
		}
	}
}