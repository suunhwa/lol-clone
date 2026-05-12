// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/InventoryViewModel.h"

#include "LeagueofLegends.h"
#include "Components/InventoryComponent.h"
#include "Item/ItemInstance.h"

void UInventoryViewModel::Initialize()
{
	if (!InventoryComp) return;

	InventoryComp->OnInventorySlotChanged.AddUObject(this, &UInventoryViewModel::HandleInventorySlotChanged);
	InventoryComp->OnGoldChanged.AddUObject(this, &UInventoryViewModel::HandleGoldChanged);
}

void UInventoryViewModel::Reset()
{
	if (InventoryComp)
	{
		InventoryComp->OnInventorySlotChanged.RemoveAll(this);
		InventoryComp->OnGoldChanged.RemoveAll(this);
	}
	
	Super::Reset();
}

void UInventoryViewModel::HandleInventorySlotChanged(int32 SlotIndex, UItemInstance* ItemInstance)
{
	if (SlotIndex == INDEX_NONE)
	{
		PRINTLOG_TK(TEXT("[InventoryViewModel] SlotIndex is INDEX_NONE. Ignoring change."));
		return;
	}
	
	FInventorySlotViewData VD;
	VD.SlotIndex = SlotIndex;
	
	if (ItemInstance) // 슬롯에 아이템이 들어온 경우
	{
		UItemDataAsset* Data = ItemInstance->GetItemData();
		VD.Icon = Data->Icon;
	}
	else // 슬롯이 비워진 경우
	{
		VD.Icon = nullptr;
	}
	
	OnItemSlotUpdated.Broadcast(VD);
}

void UInventoryViewModel::HandleGoldChanged(float NewGold)
{
	OnGoldChanged.Broadcast(NewGold);
}
