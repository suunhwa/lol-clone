#include "Characters/Minion/LoLMinion_Ranged.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"

ALoLMinion_Ranged::ALoLMinion_Ranged()
{
	MinionID = 3002; // 법사 미니언 ID
}

void ALoLMinion_Ranged::ExecuteAttack()
{
	// 부모의 로직(로그 출력 등)을 수행하고 싶다면 유지, 
	// 하지만 원거리 미니언은 투사체를 쏴야 하므로 아래 로직이 핵심입니다.
	if (CurrentTarget.IsValid())
	{
		ExecuteRangedAttack(CurrentTarget.Get());
	}
}

void ALoLMinion_Ranged::ExecuteRangedAttack(AActor* Target)
{
	// 권한 체크 및 투사체 클래스 유효성 확인
	if (!HasAuthority() || !Target || !ProjectileClass) return;

	UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp);
	if (!MinionStat) return;

	// 투사체 스폰 위치: 미니언 위치에서 약간 위(지팡이 높이)와 앞쪽으로 설정
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 60.f + FVector(0.f, 0.f, 60.f);
	FRotator SpawnRotation = (Target->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();

	// 투사체 스폰
	if (ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params))
	{
		float AD = MinionStat->GetAD();
       
		// 테이블(CSV)에서 가져온 탄속 적용
		float Speed = MinionStat->GetProjSpeed(); 
       
		// 탄속 데이터 방어 코드
		if (Speed <= 0.f) Speed = 1500.f; 

		// 팀 정보 가져오기
		ETeam MyTeam = ETeam::None;
		if (TagComp) MyTeam = TagComp->GetTeam();

		// 투사체 발사!
		Projectile->Launch(Target, Speed, AD, MyTeam);
       
		PRINTLOG_HJ(TEXT("[%s] 법사 미니언 투사체 발사! 탄속: %.1f, 데미지: %.1f"), *GetName(), Speed, AD);
	}
}