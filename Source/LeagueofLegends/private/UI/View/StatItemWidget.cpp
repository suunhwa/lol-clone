#include "UI/View/StatItemWidget.h"

#include "Components/TextBlock.h"

void UStatItemWidget::UpdateValue(float Value)
{
	if (Txt_StatValue)
	{
		Txt_StatValue->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Value)));
	}
}
