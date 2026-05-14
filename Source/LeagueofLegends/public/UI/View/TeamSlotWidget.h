#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ViewModel/PickWindowViewModel.h"
#include "TeamSlotWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class LEAGUEOFLEGENDS_API UTeamSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSlotData(const FPlayerSlotViewData& Data);
	void ClearSlot();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_nick;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_champ;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_select;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_ChampIcon;
};
