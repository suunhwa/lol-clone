#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "PlayerStatsWidget.generated.h"

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
	TObjectPtr<UStatItemWidget> Stat_AD; // 공격력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_AP; // 주문력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_Armor; // 방어력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_MR; // 마법 저항력

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_AS; // 공격 속도

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_AH; // 스킬 가속

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_Crit; // 치명타

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UStatItemWidget> Stat_MS; // 이동 속도

private:
	UPROPERTY()
	TObjectPtr<UPlayerStatsViewModel> VM;
};
