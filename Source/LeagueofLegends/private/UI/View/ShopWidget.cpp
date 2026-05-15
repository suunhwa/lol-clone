// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/ShopWidget.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
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
	Btn_Close->OnClicked.AddDynamic(this, &UShopWidget::OnCloseClicked);
	
	SetCanvasPanelHitTestInvisible();
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
	if (UShopViewModel* ShopVM = Cast<UShopViewModel>(OwnerViewModel))
	{
		ShopVM->OnShopItemsBuilt.RemoveAll(this);
		ShopVM->OnGoldUpdated.RemoveAll(this);
	}

	Super::UnbindViewModel();
}

void UShopWidget::SetSelectedSlotIndex(int32 InSlotIndex)
{
	if (!IsVisible()) return;
	SelectedSlotIndex = InSlotIndex;
}

void UShopWidget::SetCanvasPanelHitTestInvisible()
{
	if (!RootCanvas)
	{
		return;
	}

	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UShopWidget::PopulateItemList(const TArray<FItemProfileViewData>& ViewData)
{
	WrapBox_ItemList->ClearChildren();
	CurrentSelectedItem = nullptr;
	SelectedItemID = INDEX_NONE;
	
	for (const FItemProfileViewData& ItemData : ViewData)
	{
		UItemProfileWidget* NewItemProfile = CreateWidget<UItemProfileWidget>(GetOwningPlayer(), ItemProfileWidgetClass);
		WrapBox_ItemList->AddChild(NewItemProfile);

		NewItemProfile->SetItemProfile(ItemData.Icon, FString::FromInt(ItemData.Price), ItemData.ItemID);
		NewItemProfile->OnItemClicked.AddUObject(this, &UShopWidget::OnItemSlotClicked);
		NewItemProfile->BindItemButtonClick();
	}
}

void UShopWidget::OnGoldUpdated(int32 NewGold)
{
	Txt_CurrentGold->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewGold)));
}

void UShopWidget::OnItemSlotClicked(int32 ItemID, UItemProfileWidget* ClickedItem)
{
	if (CurrentSelectedItem && CurrentSelectedItem != ClickedItem)
	{
		CurrentSelectedItem->SetSelected(false);
	}
	CurrentSelectedItem = ClickedItem;
	SelectedItemID = ItemID;
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

void UShopWidget::OnCloseClicked()
{
	SetVisibility(ESlateVisibility::Hidden);
}
