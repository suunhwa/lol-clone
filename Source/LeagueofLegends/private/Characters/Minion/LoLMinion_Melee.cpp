#include "Characters/Minion/LoLMinion_Melee.h"
#include "Characters/Nexus/Nexus.h"

ALoLMinion_Melee::ALoLMinion_Melee()
{
	// 전사 미니언의 데이터 테이블 ID 설정
	MinionDataID = TEXT("3001");
}

void ALoLMinion_Melee::PerformAttack()
{
	/*// 타겟이 없으면 중단
	if (!TargetPlayer) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 공격 속도(AttackSpeed)에 따른 쿨타임 계산
	// 1.0f / AttackSpeed = 공격 간격(초)
	if (CurrentTime - LastAttackTime >= (1.0f / AttackSpeed))
	{
		// 타겟이 미니언인 경우
		if (ALoLMinion* EnemyMinion = Cast<ALoLMinion>(TargetPlayer))
		{
			EnemyMinion->TakeDamageSimple(AttackDamage);
			UE_LOG(LogTemp, Log, TEXT("[전사미니언] %s 타격! 데미지: %.1f"), *EnemyMinion->GetName(), AttackDamage);
		}
		// 타겟이 넥서스인 경우
		else if (ANexus* TargetNexus = Cast<ANexus>(TargetPlayer))
		{
			TargetNexus->ReceiveDamage(AttackDamage);
			UE_LOG(LogTemp, Warning, TEXT("[전사미니언] 넥서스 타격 중! 남은 체력: %.1f"), TargetNexus->Health);
		}

		// 마지막 공격 시간 갱신
		LastAttackTime = CurrentTime;

		// 여기서 나중에 "공격 애니메이션"을 실행
	
	}*/
	
	if (!TargetPlayer) return;
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < (1.0f / AttackSpeed)) return;

	// 타입 상관없이 IDamageable 인터페이스 하나로 처리
	if (IDamageable* DamageTarget = Cast<IDamageable>(TargetPlayer))
	{
		DamageTarget->ReceiveDamage(AttackDamage, EDamageType::Physical, this);
	}

	LastAttackTime = CurrentTime;
}