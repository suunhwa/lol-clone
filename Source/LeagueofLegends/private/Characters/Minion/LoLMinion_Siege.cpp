#include "Characters/Minion/LoLMinion_Siege.h"
#include "Characters/Minion/Ranged_Projectile.h" 
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"

ALoLMinion_Siege::ALoLMinion_Siege()
{
	MinionID = 3003; // 공성 미니언 ID 고정

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnPoint"));
	ProjectileSpawnPoint->SetupAttachment(RootComponent);
	// 대포 위치에 맞게 살짝 앞/위로 조정 (필요시 BP에서 세밀하게 조정)
	ProjectileSpawnPoint->SetRelativeLocation(FVector(80.f, 0.f, 60.f)); 
}

void ALoLMinion_Siege::ExecuteAttack()
{
	// [성공 포인트] 법사와 똑같이 부모를 부르지 않고 자신의 발사 함수를 직접 실행
	if (CurrentTarget.IsValid())
	{
		ExecuteSiegeRangedAttack(CurrentTarget.Get());
	}
}

void ALoLMinion_Siege::ExecuteSiegeRangedAttack(AActor* Target)
{
	// 법사(Ranged)의 성공 로직을 그대로 복제
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
		if (Speed <= 0.f) Speed = 1200.f; // 공성 미니언의 묵직한 포탄 속도

		ETeam MyTeam = (TagComp) ? TagComp->GetTeam() : ETeam::None;

		Projectile->Launch(Target, Speed, AD, MyTeam);
        
		PRINTLOG_HJ(TEXT("[%s] 공성 미니언 대포 발사! (데미지: %.1f)"), *GetName(), AD);
	}
}