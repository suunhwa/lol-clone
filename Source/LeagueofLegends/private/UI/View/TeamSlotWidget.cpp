#include "UI/View/TeamSlotWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTeamSlotWidget::SetSlotData(const FPlayerSlotViewData& Data)
{
	if (txt_nick)
	{
		txt_nick->SetText(FText::FromString(Data.Nickname));
	}

	const bool bChampSelected = Data.ChampionID != NAME_None;

	// 준비완료면 txt_select를 "준비완료"로 표시
	// 아니면 챔피언 선택 여부에 따라 "선택 중..." 또는 숨김
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
		txt_champ->SetVisibility(bChampSelected
			                         ? ESlateVisibility::Visible
			                         : ESlateVisibility::Collapsed);

		if (bChampSelected)
		{
			// 한글 이름 우선, 없으면 ChampionID
			const FName NameToShow = Data.ChampionName_KR.IsNone()
				? Data.ChampionID : Data.ChampionName_KR;
			txt_champ->SetText(FText::FromName(NameToShow));
		}
	}

	if (Img_ChampIcon)
	{
		// 텍스처만 교체 — 색상/불투명도는 건드리지 않아 프레임 항상 유지
		if (bChampSelected && Data.ChampPortrait)
		{
			Img_ChampIcon->SetBrushFromTexture(Data.ChampPortrait);
		}
		else
		{
			Img_ChampIcon->SetBrushFromTexture(nullptr);
		}
		Img_ChampIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UTeamSlotWidget::ClearSlot()
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
	if (Img_ChampIcon)
	{
		Img_ChampIcon->SetBrushFromTexture(nullptr);
		Img_ChampIcon->SetVisibility(ESlateVisibility::Visible);
	}
}
