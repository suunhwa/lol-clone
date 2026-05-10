// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/ItemProfileWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UItemProfileWidget::SetItemProfile(UTexture2D* Icon, const FString& Price, int32 InItemID)
{
	SetItemIcon(Icon);
	SetItemPrice(Price);
	ItemID = InItemID;
}
 
void UItemProfileWidget::SetItemIcon(UTexture2D* Icon)
{
	FButtonStyle Style = Btn_ItemProfile->GetStyle();
	Style.Normal.SetResourceObject(Icon);
	Style.Hovered.SetResourceObject(Icon);
	Style.Pressed.SetResourceObject(Icon);
	Style.Disabled.SetResourceObject(Icon);
	Btn_ItemProfile->SetStyle(Style);
}

void UItemProfileWidget::SetItemPrice(const FString& Price)
{
	Txt_Price->SetText(FText::FromString(Price));
}

void UItemProfileWidget::BindItemButtonClick()
{
	Btn_ItemProfile->OnClicked.AddDynamic(this, &UItemProfileWidget::HandleClicked);
}

void UItemProfileWidget::HandleClicked()
{
	OnItemClicked.Broadcast(ItemID);
}
