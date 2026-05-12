#include "UI/View/MainHUDWidget.h"

#include "Characters/LoLChampion.h"
#include "Components/InventoryComponent.h"
#include "UI/View/SkillBarWidget.h"
#include "UI/View/ChampionPortraitWidget.h"
#include "UI/View/GameInfoBarWidget.h"
#include "UI/View/InventoryWidget.h"
#include "UI/ViewModel/SkillBarViewModel.h"
#include "UI/ViewModel/ChampionPortraitViewModel.h"
#include "UI/ViewModel/GameInfoViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"

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
	
	if (WBP_Inventory)
	{
		UInventoryViewModel* VM = NewObject<UInventoryViewModel>(this);
		VM->SetInventoryComponent(Champion->GetComponentByClass<UInventoryComponent>());
		VM->Initialize();
		WBP_Inventory->BindViewModel(VM);
	}		
}
