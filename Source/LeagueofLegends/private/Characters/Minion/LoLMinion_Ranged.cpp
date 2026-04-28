#include "Characters/Minion/LoLMinion_Ranged.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Kismet/GameplayStatics.h"

ALoLMinion_Ranged::ALoLMinion_Ranged()
{
	MinionDataID = TEXT("3002");
}

void ALoLMinion_Ranged::PerformAttack()
{
	if (!TargetPlayer || !ProjectileClass) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime >= (1.0f / AttackSpeed))
	{
		// 1. 발사 위치 계산 (오프셋)
		FVector SpawnLocation = GetActorLocation() 
						  + (GetActorForwardVector() * ProjectileOffset.X)
						  + (GetActorUpVector() * ProjectileOffset.Z);
        
		// 2. 타겟을 향한 회전값 (바라보는 방향으로 발사)
		FRotator SpawnRotation = (TargetPlayer->GetActorLocation() - SpawnLocation).Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		// 3. 투사체 스폰
		ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			// 내 팀 태그 찾기
			FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;

			// 4. 발사! (테이블 값 ProjSpeed 사용)
			Projectile->Launch(ProjSpeed, AttackDamage, MyTeamTag);
            
			UE_LOG(LogTemp, Log, TEXT("[%s] 투사체(%s) 발사! 속도: %.1f"), *Name_KR, *Projectile->GetName(), ProjSpeed);
		}

		LastAttackTime = CurrentTime;
	}
}