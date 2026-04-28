#include "Characters/Minion/LoLMinion_Super.h"
#include "Characters/Nexus/Nexus.h"

ALoLMinion_Super::ALoLMinion_Super()
{
	MinionDataID = TEXT("3004"); // 슈퍼 미니언 ID
}

void ALoLMinion_Super::PerformAttack()
{
	if (!TargetPlayer) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime >= (1.0f / AttackSpeed))
	{
		// 내 팀 태그 확인
		FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;

		if (ALoLMinion* Enemy = Cast<ALoLMinion>(TargetPlayer))
		{
			Enemy->TakeDamageSimple(AttackDamage);
		}
		else if (class ANexus* Nexus = Cast<ANexus>(TargetPlayer))
		{
			Nexus->ReceiveDamage(AttackDamage);
		}

		LastAttackTime = CurrentTime;
		UE_LOG(LogTemp, Log, TEXT("[%s] 슈퍼 미니언 강력 타격!"), *Name_KR);
	}
}