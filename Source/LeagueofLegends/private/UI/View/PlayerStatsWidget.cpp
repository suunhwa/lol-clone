#include "UI/View/PlayerStatsWidget.h"

#include "UI/View/StatItemWidget.h"
#include "UI/ViewModel/PlayerStatsViewModel.h"

void UPlayerStatsWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);

	VM = Cast<UPlayerStatsViewModel>(InViewModel);
	if (!VM) { return; }

	VM->OnStatsRefresh.AddUObject(this, &UPlayerStatsWidget::RefreshAllStats);
	RefreshAllStats();
}

void UPlayerStatsWidget::RefreshAllStats()
{
	if (!VM) { return; }

	Stat_AD->UpdateValue(VM->GetAD());
	Stat_AP->UpdateValue(VM->GetAP());
	Stat_Armor->UpdateValue(VM->GetArmor());
	Stat_MR->UpdateValue(VM->GetMR());
	Stat_AS->UpdateValue(VM->GetAS());
	Stat_AH->UpdateValue(VM->GetAH());
	Stat_Crit->UpdateValue(VM->GetCrit());
	Stat_MS->UpdateValue(VM->GetMS());
}
