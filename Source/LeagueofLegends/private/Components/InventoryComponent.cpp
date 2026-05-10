// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryComponent.h"

#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "Item/ItemInstance.h"
#include "Manager/ItemDataSubsystem.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemSlot.Init(nullptr, MaxSlotCount);
}

void UInventoryComponent::SetGold(float NewGold)
{
	Gold = NewGold;
	OnGoldChanged.Broadcast(Gold);
}

UItemInstance* UInventoryComponent::GetItemAtSlot(int32 SlotIndex) const
{
	if (!ItemSlot.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemAtSlot: Invalid SlotIndex %d"), SlotIndex);
		return nullptr;
	}

	return ItemSlot[SlotIndex];
}

bool UInventoryComponent::PurchaseItem(int32 ItemID)
{
	UItemDataSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UItemDataSubsystem>();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("PurchaseItem: ItemDataSubsystem not found"));
		return false;
	}

	UItemDataAsset* ItemData = Subsystem->GetItemDataAsset(ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("PurchaseItem: ItemDataAsset not found for ItemID %d"), ItemID);
		return false;
	}

	// 골드 검증
	if (Gold < ItemData->Price)
	{
		UE_LOG(LogTemp, Warning, TEXT("PurchaseItem: Not enough gold. Required: %d, Available: %f"), ItemData->Price,
		       Gold);
		return false;
	}

	// 슬롯 검증
	const int EmptySlot = GetEmptySlotIndex();
	if (EmptySlot == -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("PurchaseItem: No empty slot available"));
		return false;
	}

	// 구매 처리 (골드 차감, 슬롯에 아이템 추가, 구매 내역 스택에 기록)
	SpendGold(ItemData->Price);

	UItemInstance* NewItem = NewObject<UItemInstance>(this);
	NewItem->Initialize(ItemData, GetOwner());

	ItemSlot[EmptySlot] = NewItem;
	NewItem->OnEquipped();

	FPurchaseRecord Record
	{
		.ActionType = 0, // 구매
		.ItemInstance = NewItem,
	};
	PurchaseHistoryStack.Push(Record);

	OnInventoryChanged.Broadcast();

	PRINTLOG_TK(TEXT("[%s] 구매 완료 — SlotIndex: %d / 잔여 골드: %.0f"), *ItemData->NameKR, EmptySlot, Gold);
	return true;
}

void UInventoryComponent::SellItem(int32 SlotIndex)
{
	if (!ItemSlot.IsValidIndex(SlotIndex) || !ItemSlot[SlotIndex])
	{
		PRINTLOG_TK(TEXT("유효하지 않은 슬롯: %d"), SlotIndex);
		return;
	}

	UItemInstance* ItemToSell = ItemSlot[SlotIndex];
	UItemDataAsset* ItemData = ItemToSell->GetItemData();

	AddGold(ItemData->RefundPrice);

	FPurchaseRecord Record
	{
		.ActionType = 1, // 판매
		.SlotIndex = SlotIndex,
		.ItemInstance = ItemToSell,
	};
	PurchaseHistoryStack.Push(Record);

	ItemToSell->OnUnequipped();
	ItemSlot[SlotIndex] = nullptr;

	OnInventoryChanged.Broadcast();

	PRINTLOG_TK(TEXT("[%s] 판매 완료 — SlotIndex: %d / 환급 골드: %d / 잔여 골드: %.0f"),
	            *ItemData->NameKR, SlotIndex, ItemData->RefundPrice, Gold);
}

void UInventoryComponent::Undo()
{
	if (IsHistoryEmpty())
	{
		PRINTLOG_TK(TEXT("되돌릴 내역 없음"));
		return;
	}
	
	FPurchaseRecord LastRecord = PurchaseHistoryStack.Last();
	PurchaseHistoryStack.Pop();
	
	UItemInstance* Instance = LastRecord.ItemInstance.Get();
	if (!Instance)
	{
		PRINTLOG_TK(TEXT("Undo: Instance is invalid"));
		return;
	}
	
	UItemDataAsset* ItemData = Instance->GetItemData();
	
	// 구매 취소 — 슬롯에서 제거 + 전액 환불
	if (LastRecord.ActionType == 0)
	{
		int32 FoundSlot = ItemSlot.IndexOfByKey(Instance);
		if (FoundSlot == INDEX_NONE)
		{
			PRINTLOG_TK(TEXT("Undo: 슬롯에서 Instance를 찾을 수 없음"));
			return;
		}

		ItemSlot[FoundSlot]->OnUnequipped();
		ItemSlot[FoundSlot] = nullptr;
		AddGold(ItemData->Price);

		PRINTLOG_TK(TEXT("[%s] 구매 취소 — SlotIndex: %d / 환급: %d / 잔여: %.0f"),
			*ItemData->NameKR, FoundSlot, ItemData->Price, Gold);
	}
	// 판매 취소 — 슬롯 복구 + 환급받은 골드 차감
	else if (LastRecord.ActionType == 1)
	{
		int32 TargetSlot = LastRecord.SlotIndex;

		// 원래 슬롯이 비어있지 않으면 빈 슬롯으로 대체
		if (!ItemSlot.IsValidIndex(TargetSlot) || ItemSlot[TargetSlot])
		{
			TargetSlot = GetEmptySlotIndex();
			if (TargetSlot == INDEX_NONE)
			{
				PRINTLOG_TK(TEXT("Undo: 빈 슬롯 없음"));
				return;
			}
		}

		SpendGold(ItemData->RefundPrice);
		ItemSlot[TargetSlot] = Instance;
		Instance->OnEquipped();

		PRINTLOG_TK(TEXT("[%s] 판매 취소 — SlotIndex: %d / 차감: %d / 잔여: %.0f"),
			*ItemData->NameKR, TargetSlot, ItemData->RefundPrice, Gold);
	}

	OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::EquipTrinket(UItemDataAsset* TrinketData)
{
	if (!TrinketData)
	{
		PRINTLOG_TK(TEXT("EquipTrinket: TrinketData is invalid"));
		return false;
	}

	// 기존 트링켓 해제
	if (TrinketSlot)
	{
		TrinketSlot->OnUnequipped();
		PRINTLOG_TK(TEXT("[%s] 트링켓 해제"), *TrinketSlot->GetItemData()->NameKR);
	}

	UItemInstance* NewInstance = NewObject<UItemInstance>(this);
	NewInstance->Initialize(TrinketData, GetOwner());

	TrinketSlot = NewInstance;
	TrinketSlot->OnEquipped();

	OnInventoryChanged.Broadcast();

	PRINTLOG_TK(TEXT("[%s] 트링켓 장착 완료"), *TrinketData->NameKR);
	return true;
}

int UInventoryComponent::GetEmptySlotIndex() const
{
	for (int32 i = 0; i < ItemSlot.Num(); i++)
	{
		if (ItemSlot[i] == nullptr)
		{
			return i;
		}
	}
	return INDEX_NONE; // full
}
