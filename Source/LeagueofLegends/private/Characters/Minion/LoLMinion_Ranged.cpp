#include "Characters/Minion/LoLMinion_Ranged.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"

ALoLMinion_Ranged::ALoLMinion_Ranged()
{
	MinionID = 3002;
}

void ALoLMinion_Ranged::ExecuteRangedAttack(AActor* Target)
{
	if (!HasAuthority() || !Target || !ProjectileClass) return;

	UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp);
	if (!MinionStat) return;

	// 스폰 위치 (필요 시 이 오프셋도 테이블화 가능)
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.f;
	FRotator SpawnRotation = (Target->GetActorLocation() - GetActorLocation()).Rotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();

	if (ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params))
	{
		// StatComp에 정의된 수치만 사용
		float AD = MinionStat->GetAD();
		float Speed = MinionStat->GetAttackRange(); // 보통 발사 속도는 사거리/이동속도 등과 연계되거나 별도 변수 활용
		ETeam MyTeam = TagComp->GetTeam();

		Projectile->Launch(Target, Speed, AD, MyTeam);
	}
}