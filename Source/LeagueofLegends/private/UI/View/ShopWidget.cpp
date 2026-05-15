// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/ShopWidget.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Item/ItemDataAsset.h"
#include "UI/View/ItemProfileWidget.h"
#include "UI/ViewModel/ShopViewModel.h"
#include "UI/Widget/DescStatWidget.h"

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
	
	if (SelectedItemID != INDEX_NONE)
	{
		auto* SelectedItemDataAsset = Cast<UShopViewModel>(OwnerViewModel)->GetItemDataAsset(SelectedItemID);
		SetItemDescription(SelectedItemDataAsset);
	}
	else
	{
		ResetItemDescription();
	}
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

void UShopWidget::SetItemDescription(const UItemDataAsset* ItemData)
{
	if (!ItemData) return;

	// WBP_Desc_ItemProfile
	if (WBP_Desc_ItemProfile)
	{
		WBP_Desc_ItemProfile->SetItemProfile(ItemData->Icon, FString::FromInt(ItemData->Price), ItemData->ItemID);
	}

	// Img_Desc_ItemIcon
	if (Img_Desc_ItemIcon && ItemData->Icon)
	{
		Img_Desc_ItemIcon->SetBrushFromTexture(ItemData->Icon);
	}

	// Txt_Desc_ItemName
	if (Txt_Desc_ItemName)
	{
		Txt_Desc_ItemName->SetText(FText::FromString(ItemData->NameKR));
	}

	// Txt_Desc_ItemPrice
	if (Txt_Desc_ItemPrice)
	{
		Txt_Desc_ItemPrice->SetText(FText::FromString(FString::FromInt(ItemData->Price)));
	}

	// VBox_Desc_ItemStatList - DescStatWidgetClass로 스탯 위젯 생성
	if (VBox_Desc_ItemStatList && DescStatWidgetClass)
	{
		VBox_Desc_ItemStatList->ClearChildren();

		for (const FStatModifier& Stat : ItemData->Stats)
		{
			UDescStatWidget* StatWidget = CreateWidget<UDescStatWidget>(GetOwningPlayer(), DescStatWidgetClass);
			if (!StatWidget) continue;

			const FString StatTypeName = UEnum::GetValueAsString(Stat.StatType);
			StatWidget->SetStatName(FText::FromString(StatTypeName));
			StatWidget->SetStatValue(FText::FromString(FString::Printf(TEXT("%.0f"), Stat.Value)));
			
			// StatWidget->SetDescStatWidget(FText::FromString(StatTypeName))

			VBox_Desc_ItemStatList->AddChild(StatWidget);
		}
	}

	// Txt_Desc_ItemDescription - 패시브 이펙트 설명 조합
	if (Txt_Desc_ItemDescription)
	{
		FString Combined;
		for (const FItemPassiveEffectData& Effect : ItemData->Effects)
		{
			if (!Effect.Description.IsEmpty())
			{
				if (!Combined.IsEmpty()) Combined += TEXT("\n");
				Combined += Effect.Description;
			}
		}
		Txt_Desc_ItemDescription->SetText(FText::FromString(Combined));
	}
}

void UShopWidget::ResetItemDescription()
{
	if (WBP_Desc_ItemProfile)
	{
		WBP_Desc_ItemProfile->SetItemProfile(nullptr, TEXT(""), INDEX_NONE);
	}

	if (Img_Desc_ItemIcon)
	{
		Img_Desc_ItemIcon->SetBrushFromTexture(nullptr);
	}

	if (Txt_Desc_ItemName)
	{
		Txt_Desc_ItemName->SetText(FText::GetEmpty());
	}

	if (Txt_Desc_ItemPrice)
	{
		Txt_Desc_ItemPrice->SetText(FText::GetEmpty());
	}

	if (VBox_Desc_ItemStatList)
	{
		VBox_Desc_ItemStatList->ClearChildren();
	}

	if (Txt_Desc_ItemDescription)
	{
		Txt_Desc_ItemDescription->SetText(FText::GetEmpty());
	}
}

