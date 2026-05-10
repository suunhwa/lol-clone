#include "Characters/Minion/LoLMinion_Siege.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"

ALoLMinion_Siege::ALoLMinion_Siege()
{
	MinionID = 3003; // 공성 미니언 ID 고정

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(RootComponent);
	// 위치가 너무 낮으면 바닥에 충돌할 수 있으니 살짝 위로 (필요시 BP에서 조정)
	ProjectileSpawnPoint->SetRelativeLocation(FVector(50.f, 0.f, 50.f)); 
}

void ALoLMinion_Siege::ExecuteAttack()
{
	if (CurrentTarget.IsValid())
	{
		ExecuteSiegeRangedAttack(CurrentTarget.Get());
	}
}

void ALoLMinion_Siege::ExecuteSiegeRangedAttack(AActor* Target)
{
	if (!HasAuthority() || !Target || !ProjectileClass) return;

	UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp);
	if (!MinionStat) return;

	FVector SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
	FRotator SpawnRotation = (Target->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();

	if (ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params))
	{
		float AD = MinionStat->GetAD();
		float Speed = MinionStat->GetProjSpeed();
		if (Speed <= 0.f) Speed = 1200.f; // 공성은 법사보다 조금 느릴 수 있음

		ETeam MyTeam = (TagComp) ? TagComp->GetTeam() : ETeam::None;

		Projectile->Launch(Target, Speed, AD, MyTeam);
		PRINTLOG_HJ(TEXT("[%s] 공성 미니언 대포 발사! 데미지: %.1f"), *GetName(), AD);
	}
}