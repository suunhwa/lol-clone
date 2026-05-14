#include "UI/View/PlayerStatsWidget.h"

#include "Components/TextBlock.h"
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
	
	Txt_StatAD->SetText(FText::FromString(FString::Printf(TEXT("%d"), static_cast<int32>(VM->GetAD()))));
	Txt_StatAP->SetText(FText::FromString(FString::Printf(TEXT("%d"), static_cast<int32>(VM->GetAP()))));
	Txt_StatArmor->SetText(FText::FromString(FString::Printf(TEXT("%d"), static_cast<int32>(VM->GetArmor()))));
	Txt_StatMR->SetText(FText::FromString(FString::Printf(TEXT("%d"), static_cast<int32>(VM->GetMR()))));
	Txt_StatAS->SetText(FText::FromString(FString::Printf(TEXT("%.2f"), VM->GetAS())));
	Txt_StatAH->SetText(FText::FromString(FString::Printf(TEXT("%d"), static_cast<int32>(VM->GetAH()))));
	Txt_StatCrit->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), static_cast<int32>(VM->GetCrit()) * 100)));
	Txt_StatMS->SetText(FText::FromString(FString::Printf(TEXT("%d"), static_cast<int32>(VM->GetMS()))));
}
