// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/ItemProfileWidget.h"

#include "Components/CheckBox.h"
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
	Img_Icon->SetBrushFromTexture(Icon);
}

void UItemProfileWidget::SetItemPrice(const FString& Price)
{
	Txt_Price->SetText(FText::FromString(Price));
}

void UItemProfileWidget::BindItemButtonClick()
{
	Tgl_ItemProfile->OnCheckStateChanged.AddDynamic(this, &UItemProfileWidget::HandleToggleStateChanged);
}

void UItemProfileWidget::SetSelected(bool bSelected)
{
	Tgl_ItemProfile->SetCheckedState(bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
	HandleToggleStateChanged(bSelected);
}

void UItemProfileWidget::HandleToggleStateChanged(bool bIsChecked)
{
	const float Alpha = bIsChecked ? 1.f : 0.f;
	Img_OutlineL->SetRenderOpacity(Alpha);
	Img_OutlineT->SetRenderOpacity(Alpha);
	Img_OutlineR->SetRenderOpacity(Alpha);
	Img_OutlineB->SetRenderOpacity(Alpha);

	if (bIsChecked)
	{
		OnItemClicked.Broadcast(ItemID, this);
	}
}
