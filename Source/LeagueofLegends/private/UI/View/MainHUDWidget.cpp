#include "UI/View/MainHUDWidget.h"

#include "Characters/LoLChampion.h"
#include "Components/InventoryComponent.h"
#include "Components/Widget.h"
#include "FOW/FOWManager.h"
#include "FOW/FOWTileMap.h"
#include "GameFramework/RiftGameState.h"
#include "UI/View/SkillBarWidget.h"
#include "UI/View/ChampionPortraitWidget.h"
#include "UI/View/GameInfoBarWidget.h"
#include "UI/View/InventoryWidget.h"
#include "UI/View/MinimapWidget.h"
#include "UI/View/PlayerStatsWidget.h"
#include "UI/ViewModel/SkillBarViewModel.h"
#include "UI/ViewModel/ChampionPortraitViewModel.h"
#include "UI/ViewModel/GameInfoViewModel.h"
#include "UI/ViewModel/InventoryViewModel.h"
#include "UI/ViewModel/PlayerStatsViewModel.h"

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

	if (PlayerStats)
	{
		UPlayerStatsViewModel* VM = NewObject<UPlayerStatsViewModel>(this);
		VM->Setup(Champion);
		PlayerStats->BindViewModel(VM);
		PlayerStats->SetVisibility(ESlateVisibility::Hidden);
	}

	if (ChampionPortrait)
	{
		ChampionPortrait->OnStatsToggleRequested.AddUObject(this, &UMainHUDWidget::OnStatsToggleRequested);
	}

	if (AFOWManager* FOW = GS->GetFOWManager())
	{
		AFOWTileMap* LocalTileMap = FOW->GetLocalTileMap();
		
		if (LocalTileMap)
		{
			WBP_Minimap->SetLocalTileMap(LocalTileMap);
		}

		FOW->OnFOWReady.AddLambda([this](UTexture2D* Tex)
		{
			if (WBP_Minimap && Tex)
			{
				WBP_Minimap->SetMinimapFOWTexture(Tex);
			}
		});
	}
 }

void UMainHUDWidget::OnStatsToggleRequested()
{
	if (!PlayerStats) { return; }

	const bool bVisible = PlayerStats->GetVisibility() == ESlateVisibility::Visible;
	PlayerStats->SetVisibility(bVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}
