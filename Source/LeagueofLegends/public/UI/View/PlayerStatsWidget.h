#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "PlayerStatsWidget.generated.h"

class UTextBlock;
class UStatItemWidget;
class UPlayerStatsViewModel;

UCLASS()
class LEAGUEOFLEGENDS_API UPlayerStatsWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;

private:
	UFUNCTION()
	void RefreshAllStats();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatAD; // 공격력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatAP; // 주문력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatArmor; // 방어력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatMR; // 마법 저항력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatAS; // 공격 속도

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatAH; // 스킬 가속

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatCrit; // 치명타

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatMS; // 이동 속도

private:
	UPROPERTY()
	TObjectPtr<UPlayerStatsViewModel> VM;
};
