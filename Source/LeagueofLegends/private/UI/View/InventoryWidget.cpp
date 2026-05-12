// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/InventoryWidget.h"

#include "LeagueofLegends.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "UI/View/ItemSlotWidget.h"
#include "UI/ViewModel/InventoryViewModel.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitSlots();
}

void UInventoryWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);
	
	auto* InventoryViewModel = Cast<UInventoryViewModel>(OwnerViewModel);
	if (!InventoryViewModel)
	{
		PRINTLOG_TK(TEXT("Invalid ViewModel type: %s"), *GetNameSafe(InViewModel));
		return;
	}
	
	InventoryViewModel->OnItemSlotUpdated.AddUObject(this, &UInventoryWidget::HandleSlotChanged);
	InventoryViewModel->OnGoldChanged.AddUObject(this, &UInventoryWidget::HandleGoldChanged);
}

void UInventoryWidget::UnbindViewModel()
{
	Super::UnbindViewModel();
}

void UInventoryWidget::HandleSlotChanged(const FInventorySlotViewData& SlotData)
{
	if (SlotData.SlotIndex < 0 || SlotData.SlotIndex >= SlotWidgets.Num())
	{
		PRINTLOG_TK(TEXT("Invalid SlotIndex: %d"), SlotData.SlotIndex);
		return;
	}
	
	UItemSlotWidget* SlotWidget = SlotWidgets[SlotData.SlotIndex];
	if (SlotData.Icon)
	{
		SlotWidget->SetItemIcon(SlotData.Icon);
		SlotWidgets[SlotData.SlotIndex]->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SlotWidget->SetItemIcon(nullptr);
		SlotWidgets[SlotData.SlotIndex]->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryWidget::HandleGoldChanged(int32 NewGold)
{
	Txt_CurrentGold->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewGold)));
}

void UInventoryWidget::InitSlots()
{
	TArray<UScaleBox*> ScaleBoxes =
	{
		ScaleBox_Slot0, ScaleBox_Slot1, ScaleBox_Slot2,
		ScaleBox_Slot3, ScaleBox_Slot4, ScaleBox_Slot5
	};
	
	for (int32 i = 0; i < ScaleBoxes.Num(); ++i)
	{
		UScaleBox* Box = ScaleBoxes[i];
		
		UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), ItemSlotWidgetClass);
		Box->AddChild(SlotWidget);

		SlotWidget->SetSlotIndex(i + 1);
		
		SlotWidgets.Add(SlotWidget);
		SlotWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
