#include "GameFramework/RiftHUD.h"

#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UI/View/MainHUDWidget.h"
#include "UI/View/SkillBarWidget.h"

ARiftHUD::ARiftHUD()
{
}

void ARiftHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ARiftHUD::InitHUD(ALoLChampion* Champion)
{
	// BP에서 이미 위젯을 만든 경우를 대비해 lazy 생성
	if (!MainHUDWidget && MainHUDClass)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			MainHUDWidget = CreateWidget<UMainHUDWidget>(PC, MainHUDClass);
			if (MainHUDWidget)
				MainHUDWidget->AddToViewport();
		}
	}

	PRINTLOG_SH(TEXT("[RiftHUD] InitHUD. Widget=%s Champion=%s"),
		*GetNameSafe(MainHUDWidget), *GetNameSafe(Champion));

	if (!MainHUDWidget || !Champion) return;

	APlayerController* PC = GetOwningPlayerController();
	ARiftPlayerState* PS = PC ? PC->GetPlayerState<ARiftPlayerState>() : nullptr;
	ARiftGameState*   GS = GetWorld()->GetGameState<ARiftGameState>();

	PRINTLOG_SH(TEXT("[RiftHUD] PS=%s GS=%s"), *GetNameSafe(PS), *GetNameSafe(GS));

	MainHUDWidget->InitHUD(Champion, PS, GS);
}

void ARiftHUD::RefreshSkillIcons(ALoLChampion* Champion)
{
	if (!MainHUDWidget || !Champion || !Champion->GetChampionData()) return;
	if (USkillBarWidget* Bar = MainHUDWidget->GetSkillBar())
		Bar->RefreshIcons(Champion->GetChampionData());
}
