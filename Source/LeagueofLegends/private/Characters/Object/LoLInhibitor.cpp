
#include "Characters/Object/LoLInhibitor.h"

ALoLInhibitor::ALoLInhibitor()
{
	Health = 1500.f;
	MaxHealth = 1500.f;
	RespawnTime = 300.f; // 5분
}

void ALoLInhibitor::OnDestroyed()
{
	Super::OnDestroyed();
    
	UE_LOG(LogTemp, Warning, TEXT("[%s] 억제기 파괴됨! %f초 후 재생성됩니다."), *GetName(), RespawnTime);
    
	// 나중에 여기서 재생성 타이머를 돌리거나 슈퍼 미니언 스폰 신호를 보냅니다.
}

void ALoLInhibitor::Respawn()
{
	// 재생성 로직
}