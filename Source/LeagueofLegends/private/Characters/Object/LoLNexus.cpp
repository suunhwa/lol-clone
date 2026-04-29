#include "Characters/Object/LoLNexus.h"


ALolNexus::ALolNexus()
{
	Health = 3000.f;
	MaxHealth = 3000.f;
}

void ALolNexus::OnDestroyed()
{
	Super::OnDestroyed();
    
	// 게임 종료 로직 트리거
	UE_LOG(LogTemp, Error, TEXT("!!! %s 파괴됨 - 게임 종료 !!!"), (Team == ETeam::Red ? TEXT("레드팀") : TEXT("블루팀")));
}