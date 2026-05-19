#include "UI/ViewModel/ChampionPortraitViewModel.h"

#include "Characters/LoLChampion.h"
#include "Characters/Data/ChampionData.h"
#include "Components/StatComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/RiftPlayerState.h"

// LoL 레벨별 필요 경험치 (레벨 1→2부터 17→18까지)
static const float GXPRequiredPerLevel[] =
{
	280, 380, 480, 580, 680, 780, 880,
	1080, 1280, 1480, 1780, 2080, 2380,
	2680, 2980, 3280, 3480
};

void UChampionPortraitViewModel::Setup(ALoLChampion* InChampion)
{
	Champion = InChampion;
	if (!Champion || !Champion->StatComp) return;

	Champion->StatComp->OnLevelChanged.AddUObject(this, &UChampionPortraitViewModel::HandleLevelChanged);

	// PlayerState XP 구독 (폴링으로 처리 — PS에 XP 델리게이트 없음)
	PlayerState = Cast<ARiftPlayerState>(Champion->GetPlayerState());
}

int32 UChampionPortraitViewModel::GetLevel() const
{
	return (Champion && Champion->StatComp) ? Champion->StatComp->GetLevel() : 1;
}

UTexture2D* UChampionPortraitViewModel::GetPortraitTexture() const
{
	UChampionData* Data = Champion ? Champion->GetChampionData() : nullptr;
	return Data ? Data->PortraitTexture : nullptr;
}

float UChampionPortraitViewModel::GetXPProgress() const
{
	if (!PlayerState) return 0.f;

	const int32 Level = GetLevel();
	if (Level >= 18) return 1.f;

	const float Required = GXPRequiredPerLevel[Level - 1]; // 현재 레벨 → 다음 레벨 필요 XP
	if (Required <= 0.f) return 0.f;

	return FMath::Clamp(PlayerState->GetXP() / Required, 0.f, 1.f);
}

bool UChampionPortraitViewModel::IsChampionDead() const
{
	return PlayerState ? PlayerState->IsDead() : false;
}

float UChampionPortraitViewModel::GetRespawnTimeRemaining() const
{
	if (!PlayerState || !PlayerState->IsDead()) { return 0.f; }
	if (!Champion) { return 0.f; }

	const AGameStateBase* GS = Champion->GetWorld()->GetGameState<AGameStateBase>();
	if (!GS) { return 0.f; }

	return FMath::Max(0.f, PlayerState->GetRespawnEndServerTime() - GS->GetServerWorldTimeSeconds());
}

void UChampionPortraitViewModel::HandleLevelChanged(int32 NewLevel)
{
	OnLevelUpdated.Broadcast(NewLevel);
}
