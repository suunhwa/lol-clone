#include "Characters/Minion/LoLMinion_Siege.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"

ALoLMinion_Siege::ALoLMinion_Siege()
{
	MinionID = 3003;
}

void ALoLMinion_Siege::ExecuteRangedAttack(AActor* Target)
{
	if (!HasAuthority() || !Target || !ProjectileClass) return;

	UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp);
	if (!MinionStat) return;

	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 80.f + FVector(0.f, 0.f, 40.f);
	FRotator SpawnRotation = (Target->GetActorLocation() - GetActorLocation()).Rotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();

	if (ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params))
	{
		// 데이터 테이블(StatComp) 수치 주입
		float AD = MinionStat->GetAD();
		float Speed = MinionStat->GetAttackRange(); // 테이블의 데이터를 기반으로 작동
		ETeam MyTeam = TagComp->GetTeam();

		Projectile->Launch(Target, Speed, AD, MyTeam);
	}
}