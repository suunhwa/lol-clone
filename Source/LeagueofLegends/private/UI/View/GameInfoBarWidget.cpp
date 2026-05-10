#include "UI/View/GameInfoBarWidget.h"

#include "Components/TextBlock.h"
#include "UI/ViewModel/GameInfoViewModel.h"

extern ENGINE_API float GAverageFPS;

void UGameInfoBarWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);
	VM = Cast<UGameInfoViewModel>(InViewModel);
	Refresh();
}

void UGameInfoBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TickAccum += InDeltaTime;
	if (TickAccum < RefreshInterval) return;
	TickAccum = 0.f;

	Refresh();
}

void UGameInfoBarWidget::Refresh()
{
	if (VM)
	{
		if (Txt_BlueKills)
			Txt_BlueKills->SetText(FText::AsNumber(VM->GetBlueKills()));

		if (Txt_RedKills)
			Txt_RedKills->SetText(FText::AsNumber(VM->GetRedKills()));

		if (Txt_KDA)
			Txt_KDA->SetText(FText::FromString(FString::Printf(
				TEXT("%d / %d / %d"), VM->GetKills(), VM->GetDeaths(), VM->GetAssists())));

		if (Txt_CS)
			Txt_CS->SetText(FText::AsNumber(VM->GetCS()));

		if (Txt_GameTime)
		{
			const int32 Total = VM->GetElapsedSeconds();
			const int32 Minutes = Total / 60;
			const int32 Seconds = Total % 60;
			Txt_GameTime->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
		}
	}

	if (Txt_FPS)
		Txt_FPS->SetText(FText::FromString(FString::Printf(TEXT("FPS: %d"), FMath::RoundToInt(GAverageFPS))));

	if (Txt_Ping && VM)
	{
		const float PingMs = VM->GetPingMs(GetOwningPlayer());
		Txt_Ping->SetText(FText::FromString(FString::Printf(TEXT("%.0f ms"), PingMs)));
	}
}
