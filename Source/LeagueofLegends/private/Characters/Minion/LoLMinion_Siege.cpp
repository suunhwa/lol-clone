#include "Characters/Minion/LoLMinion_Siege.h"
#include "Characters/Minion/Ranged_Projectile.h"

ALoLMinion_Siege::ALoLMinion_Siege()
{
	MinionDataID = TEXT("3003"); // 공성 미니언 ID
}

void ALoLMinion_Siege::PerformAttack()
{
	if (!TargetPlayer || !ProjectileClass) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAttackTime >= (1.0f / AttackSpeed))
	{
		FVector SpawnLocation = GetActorLocation() 
						  + (GetActorForwardVector() * ProjectileOffset.X)
						  + (GetActorUpVector() * ProjectileOffset.Z);
        
		FRotator SpawnRotation = (TargetPlayer->GetActorLocation() - SpawnLocation).Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			// 태그 컴포넌트 대신 Actor Tag 사용
			FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;
			Projectile->Launch(ProjSpeed, AttackDamage, MyTeamTag);
            
			UE_LOG(LogTemp, Log, TEXT("[%s] 공성 포탄 발사!"), *Name_KR);
		}

		LastAttackTime = CurrentTime;
	}
}