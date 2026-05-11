#include "Characters/Minion/LoLMinion_Super.h"

#include "LeagueofLegends.h"
#include "Components/CapsuleComponent.h"

ALoLMinion_Super::ALoLMinion_Super()
{
	MinionID = 3004; // 슈퍼 미니언 ID 고정

	// 전사보다 덩치가 크니까 캡슐 크기 세팅 유지
	if (auto* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleHalfHeight(100.f);
		Capsule->SetCapsuleRadius(60.f);
	}
}

void ALoLMinion_Super::BeginPlay()
{
	Super::BeginPlay();
	// 성공 확인을 위한 로그
	PRINTLOG_HJ(TEXT("[슈퍼 미니언] %s 소환 및 스탯 로드 완료!"), *GetName());
}

void ALoLMinion_Super::ExecuteAttack()
{
	// [성공 포인트] 전사와 똑같이 부모(ALoLMinion)의 근접 공격 로직을 호출
	// 이렇게 하면 부모 코드에 있는 ApplyDamage가 실행됨
	Super::ExecuteAttack();
}