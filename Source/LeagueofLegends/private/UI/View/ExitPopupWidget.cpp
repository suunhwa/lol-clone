#include "UI/View/ExitPopupWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

// 간단해서 mvvm구조로 안했어요

void UExitPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UExitPopupWidget::OnClickExit);
	}
	
	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UExitPopupWidget::OnClickClose);
	}
}

void UExitPopupWidget::OnClickExit()
{
	// 로비 맵으로 이동
	UGameplayStatics::OpenLevel(this, FName("Lv_Lobby"));
}

void UExitPopupWidget::OnClickClose()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
