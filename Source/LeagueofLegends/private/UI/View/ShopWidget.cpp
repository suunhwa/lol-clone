// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/ShopWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "UI/View/ItemProfileWidget.h"
#include "UI/ViewModel/ShopViewModel.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Btn_Purchase->OnClicked.AddDynamic(this, &UShopWidget::OnPurchaseClicked);
	Btn_Sell->OnClicked.AddDynamic(this, &UShopWidget::OnSellClicked);
	Btn_Undo->OnClicked.AddDynamic(this, &UShopWidget::OnUndoClicked);
}

void UShopWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);
	
	if (UShopViewModel* ShopVM = Cast<UShopViewModel>(InViewModel))
	{
		ShopVM->OnShopItemsBuilt.AddUObject(this, &UShopWidget::PopulateItemList);
		ShopVM->OnGoldUpdated.AddUObject(this, &UShopWidget::OnGoldUpdated);
	}
}

void UShopWidget::UnbindViewModel()
{
	Super::UnbindViewModel();
}

void UShopWidget::PopulateItemList(const TArray<FItemProfileViewData>& ViewData)
{
	WrapBox_ItemList->ClearChildren();
	
	for (const FItemProfileViewData& ItemData : ViewData)
	{
		UItemProfileWidget* NewItemProfile = CreateWidget<UItemProfileWidget>(GetOwningPlayer(), ItemProfileWidgetClass);
		NewItemProfile->SetItemProfile(ItemData.Icon, FString::FromInt(ItemData.Price), ItemData.ItemID);
		WrapBox_ItemList->AddChild(NewItemProfile);
	}
}

void UShopWidget::OnGoldUpdated(int32 NewGold)
{
	Txt_CurrentGold->SetText(FText::FromString(FString::Printf(TEXT("Gold: %d"), NewGold)));
}

void UShopWidget::OnItemSlotClicked(int32 ItemID)
{
}

void UShopWidget::OnPurchaseClicked()
{
	auto* ShopVM = Cast<UShopViewModel>(OwnerViewModel);
	if (ShopVM && SelectedItemID != INDEX_NONE)
	{
		ShopVM->RequestPurchase(SelectedItemID);
	}
}

void UShopWidget::OnSellClicked()
{
	auto* ShopVM = Cast<UShopViewModel>(OwnerViewModel);
	if (ShopVM && SelectedSlotIndex != INDEX_NONE)
	{
		ShopVM->RequestSell(SelectedSlotIndex);
	}
}

void UShopWidget::OnUndoClicked()
{
	auto* ShopVM = Cast<UShopViewModel>(OwnerViewModel);
	if (ShopVM)
	{
		ShopVM->RequestUndo();
	}
}
