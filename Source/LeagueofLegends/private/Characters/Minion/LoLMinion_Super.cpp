#include "Characters/Minion/LoLMinion_Super.h"

#include "LeagueofLegends.h"
#include "Components/CapsuleComponent.h"

ALoLMinion_Super::ALoLMinion_Super()
{
	MinionID = 3004; // 슈퍼 미니언 ID 고정

	// 덩치 설정
	if (auto* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleHalfHeight(100.f);
		Capsule->SetCapsuleRadius(60.f);
	}
}

void ALoLMinion_Super::BeginPlay()
{
	Super::BeginPlay();
	PRINTLOG_HJ(TEXT("[슈퍼 미니언] %s 소환 완료 (ID: %d)"), *GetName(), MinionID);
}

void ALoLMinion_Super::ExecuteAttack()
{
	// 전사(Melee)와 똑같이 부모의 공격 로직을 명시적으로 호출
	Super::ExecuteAttack();
}