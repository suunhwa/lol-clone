#include "UI/View/MainHUDWidget.h"

#include "UI/View/SkillBarWidget.h"
#include "UI/View/ChampionPortraitWidget.h"
#include "UI/View/GameInfoBarWidget.h"
#include "UI/ViewModel/SkillBarViewModel.h"
#include "UI/ViewModel/ChampionPortraitViewModel.h"
#include "UI/ViewModel/GameInfoViewModel.h"

void UMainHUDWidget::InitHUD(ALoLChampion* Champion, ARiftPlayerState* PS, ARiftGameState* GS)
{
	if (SkillBar)
	{
		USkillBarViewModel* VM = NewObject<USkillBarViewModel>(this);
		VM->Setup(Champion);
		SkillBar->BindViewModel(VM);
	}

	if (ChampionPortrait)
	{
		UChampionPortraitViewModel* VM = NewObject<UChampionPortraitViewModel>(this);
		VM->Setup(Champion);
		ChampionPortrait->BindViewModel(VM);
	}

	if (GameInfoBar)
	{
		UGameInfoViewModel* VM = NewObject<UGameInfoViewModel>(this);
		VM->Setup(PS, GS);
		GameInfoBar->BindViewModel(VM);
	}
}
