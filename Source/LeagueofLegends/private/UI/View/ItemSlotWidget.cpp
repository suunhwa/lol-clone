// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/ItemSlotWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UItemSlotWidget::SetItemProfile(UTexture2D* Icon, const FString& Price, int32 InItemID)
{
}

void UItemSlotWidget::SetItemIcon(UTexture2D* Icon)
{
	FButtonStyle Style = Btn_Icon->GetStyle();
	Style.Normal.SetResourceObject(Icon);
	Style.Hovered.SetResourceObject(Icon);
	Style.Pressed.SetResourceObject(Icon);
	Style.Disabled.SetResourceObject(Icon);
	Btn_Icon->SetStyle(Style);
}

void UItemSlotWidget::SetSlotIndex(int32 InSlotIndex)
{
	Txt_Index->SetText(FText::AsNumber(InSlotIndex));
}

void UItemSlotWidget::BindSlotButtonClick()
{
	Btn_Icon->OnClicked.AddDynamic(this, &UItemSlotWidget::HandleSlotButtonClicked);
}

void UItemSlotWidget::HandleSlotButtonClicked()
{
	OnSlotClicked.Broadcast(SlotIndex);
}

