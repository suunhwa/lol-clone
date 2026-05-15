// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/ShopViewModel.h"

#include "LeagueofLegends.h"
#include "Components/InventoryComponent.h"
#include "Manager/ItemDataSubsystem.h"

void UShopViewModel::Initialize()
{
	if (!ItemSubsystem)	
	{
		PRINTLOG_TK(TEXT("ItemDataSubsystem is not set!"));
		return;
	}
	
	if (!InventoryComp)
	{
		PRINTLOG_TK(TEXT("InventoryComponent is not set!"));
		return;
	}
	
	InventoryComp->OnGoldChanged.AddUObject(this, &UShopViewModel::HandleGoldChanged);
	BuildViewData();
}

void UShopViewModel::Reset()
{
	Super::Reset();
}

UItemDataAsset* UShopViewModel::GetItemDataAsset(const int32 ItemID) const
{
	if (!ItemSubsystem)
	{
		PRINTLOG_TK(TEXT("Cannot get item data asset because ItemDataSubsystem is not set!"));
		return nullptr;
	}
	
	return ItemSubsystem->GetItemDataAssetByID(ItemID);
}

UTexture2D* UShopViewModel::GetItemStatIcon(ELolStatType StatType) const
{
	if (!ItemSubsystem)
	{
		PRINTLOG_TK(TEXT("Cannot get item stat icon because ItemDataSubsystem is not set!"));
		return nullptr;
	}
	
	return ItemSubsystem->GetStatIcon(StatType);
}

FString UShopViewModel::GetStatNameKR(ELolStatType StatType) const
{
	if (!ItemSubsystem)
	{
		PRINTLOG_TK(TEXT("Cannot get stat name because ItemDataSubsystem is not set!"));
		return FString("Unknown");
	}
	
	return ItemSubsystem->GetStatNameKR(StatType);
}

void UShopViewModel::RequestPurchase(int32 ItemID)
{
	if (InventoryComp)
	{
		bool bSuccess = InventoryComp->PurchaseItem(ItemID);
		if (bSuccess)
		{
			OnGoldUpdated.Broadcast(InventoryComp->GetGold());
		}
	}
}

void UShopViewModel::RequestSell(int32 SlotIndex)
{
	if (InventoryComp)
	{
		InventoryComp->SellItem(SlotIndex);
		OnGoldUpdated.Broadcast(InventoryComp->GetGold());
	}
}

void UShopViewModel::RequestUndo()
{
	if (InventoryComp && !InventoryComp->IsHistoryEmpty())
	{
		InventoryComp->Undo();
		OnGoldUpdated.Broadcast(InventoryComp->GetGold());
	}
}

void UShopViewModel::BuildViewData()
{
	if (!ItemSubsystem)
	{
		PRINTLOG_TK(TEXT("Cannot build view data because ItemDataSubsystem is not set!"));
	}
	
	TArray<FItemProfileViewData> ViewData;
	for (auto& [ID, DataAsset] : ItemSubsystem->GetAllItemDataAssets())
	{
		FItemProfileViewData Data
		{
			.ItemID = ID,
			.Name = FText::FromString(DataAsset->NameKR),
			.Price = DataAsset->Price,
			.Icon = DataAsset->Icon
		};
		
		ViewData.Add(Data);
	}
	
	OnShopItemsBuilt.Broadcast(ViewData);
}

void UShopViewModel::HandleGoldChanged(float NewGold)
{
	const int32 GoldInt = static_cast<int32>(NewGold);
	OnGoldUpdated.Broadcast(GoldInt);
}
