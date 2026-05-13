#include "UI/View/EnemySlotWidget.h"

#include "Components/TextBlock.h"

void UEnemySlotWidget::SetSlotData(const FPlayerSlotViewData& Data)
{
	if (txt_nick)
	{
		txt_nick->SetText(FText::FromString(Data.Nickname));
	}

	const bool bChampSelected = Data.ChampionID != NAME_None;

	if (txt_select)
	{
		if (Data.bIsReady)
		{
			txt_select->SetVisibility(ESlateVisibility::Visible);
			txt_select->SetText(FText::FromString(TEXT("준비완료")));
		}
		else
		{
			txt_select->SetVisibility(bChampSelected
				                          ? ESlateVisibility::Collapsed
				                          : ESlateVisibility::Visible);

			if (!bChampSelected)
			{
				txt_select->SetText(FText::FromString(TEXT("선택 중...")));
			}
		}
	}

	if (txt_champ)
	{
		txt_champ->SetVisibility(bChampSelected && !Data.bIsReady
			                         ? ESlateVisibility::Visible
			                         : ESlateVisibility::Collapsed);

		if (bChampSelected)
		{
			txt_champ->SetText(FText::FromName(Data.ChampionID));
		}
	}
}

void UEnemySlotWidget::ClearSlot()
{
	if (txt_nick)
	{
		txt_nick->SetText(FText::GetEmpty());
	}
	
	if (txt_champ)
	{
		txt_champ->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (txt_select)
	{
		txt_select->SetVisibility(ESlateVisibility::Visible);
		txt_select->SetText(FText::FromString(TEXT("선택 중...")));
	}
}
