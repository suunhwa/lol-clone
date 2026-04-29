#include "Characters/LoLStructure.h"

ALoLStructure::ALoLStructure()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALoLStructure::BeginPlay()
{
	Super::BeginPlay();

	// [중요] 기존 에디터에서 수동으로 넣은 태그가 인덱스를 꼬이게 할 수 있으므로 정리 후 재설정
	Tags.Empty();

	// 1. 팀 태그 설정 (Index 0) -> 미니언의 MyTeamTag / EnemyTeamTag와 매칭됨
	FName TeamTag = (Team == ETeam::Red) ? TEXT("RedTeam") : TEXT("BlueTeam");
	Tags.Add(TeamTag);

	// 2. 공통 식별 태그 설정 (Index 1) -> 미니언의 GetAllActorsWithTag("Character")와 매칭됨
	Tags.Add(TEXT("Character"));

	UE_LOG(LogTemp, Log, TEXT("[%s] 구조물 초기화 완료 - 팀: %s"), *GetName(), *TeamTag.ToString());
}

void ALoLStructure::ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
	if (bIsDestroyed) return;

	Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
    
	// 구조물 체력 상황 로그
	UE_LOG(LogTemp, Log, TEXT("[%s] 구조물 피격! 남은 체력: %.1f"), *GetName(), Health);

	if (Health <= 0.f)
	{
		OnDestroyed();
	}
}

void ALoLStructure::OnDestroyed()
{
	if (bIsDestroyed) return;
	bIsDestroyed = true;

	// 미니언들이 더 이상 타겟팅하지 못하도록 태그 제거
	Tags.Empty();
    
	UE_LOG(LogTemp, Error, TEXT("[%s] 구조물 파괴됨!"), *GetName());

	// 여기서 파괴 이펙트 실행 및 액터 제거 로직 추가
	// SetLifeSpan(0.1f); 
}