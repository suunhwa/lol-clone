// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryComponent.h"

#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "Item/ItemInstance.h"
#include "Manager/ItemDataSubsystem.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	ItemSlot.Init(nullptr, MaxSlotCount);
	ReplicatedSlotIDs.Init(-1, MaxSlotCount);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 내 인벤토리는 나만 받으면 됨
	DOREPLIFETIME_CONDITION(UInventoryComponent, ReplicatedSlotIDs, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryComponent, ReplicatedTrinketID, COND_OwnerOnly);}

void UInventoryComponent::SetGold(float NewGold)
{
	// 서버가 아니면 	
	if (!GetOwner()->HasAuthority()) 
	{
		return;
	}
	
	Gold = NewGold;
	OnGoldChanged.Broadcast(Gold);
	
	ClientRPC_SyncGold(Gold);
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
	if (!GetOwner()->HasAuthority())
	{
		// Client -> Server 구매 요청
		ServerRPC_PurchaseItem(ItemID);
		return false;
	}
	
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
		UE_LOG(LogTemp, Warning, TEXT("PurchaseItem: Not enough gold. Required: %d, Available: %f"), ItemData->Price, Gold);
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
	
	// Host(Server) UI 갱신 - OnRep는 클라이언트에서만 호출되므로 직접 처리
	OnInventorySlotChanged.Broadcast(EmptySlot, NewItem);
	
	// ReplicatedSlotIDs 갱신 -> OwningClient에 자동 복제
	SyncSlotIDsToClients();

	FPurchaseRecord Record
	{
		.ActionType = 0, // 구매
		.ItemInstance = NewItem,
	};
	PurchaseHistoryStack.Push(Record);

	PRINTLOG_TK(TEXT("[%s] 구매 완료 — SlotIndex: %d / 잔여 골드: %.0f"), *ItemData->NameKR, EmptySlot, Gold);
	return true;
}

void UInventoryComponent::SellItem(int32 SlotIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		ServerRPC_SellItem(SlotIndex);
		return;
	}
	
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
	
	// Host UI 갱신 
	OnInventorySlotChanged.Broadcast(SlotIndex, nullptr);
	
	SyncSlotIDsToClients();

	PRINTLOG_TK(TEXT("[%s] 판매 완료 — SlotIndex: %d / 환급 골드: %d / 잔여 골드: %.0f"),
	            *ItemData->NameKR, SlotIndex, ItemData->RefundPrice, Gold);
}

void UInventoryComponent::Undo()
{
	if (!GetOwner()->HasAuthority())
	{
		ServerRPC_Undo();
		return;
	}
	
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
		OnInventorySlotChanged.Broadcast(FoundSlot, nullptr);
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
		OnInventorySlotChanged.Broadcast(TargetSlot, Instance);
		Instance->OnEquipped();

		PRINTLOG_TK(TEXT("[%s] 판매 취소 — SlotIndex: %d / 차감: %d / 잔여: %.0f"),
			*ItemData->NameKR, TargetSlot, ItemData->RefundPrice, Gold);
	}

	SyncSlotIDsToClients();
}

bool UInventoryComponent::EquipTrinket(UItemDataAsset* TrinketData)
{
	if (!GetOwner()->HasAuthority())
	{
		// Client -> Server 장착 요청
		// ServerRPC_EquipTrinket(TrinketData ? TrinketData->ItemID : -1);
		return false;
	}
	
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

	UItemInstance* NewItem = NewObject<UItemInstance>(this);
	NewItem->Initialize(TrinketData, GetOwner());

	TrinketSlot = NewItem;
	TrinketSlot->OnEquipped();

	// OnInventoryChanged.Broadcast();
	ReplicatedTrinketID = TrinketData->ItemID;
	
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

void UInventoryComponent::SyncSlotIDsToClients()
{
	for (int32 i = 0; i < ItemSlot.Num(); i++)
	{
		ReplicatedSlotIDs[i] = ItemSlot[i]
			? ItemSlot[i]->GetItemData()->ItemID
			: -1;
	}
}

void UInventoryComponent::RebuildSlotFromIDs()
{
	UItemDataSubsystem* Subsystem =
	GetWorld()->GetGameInstance()->GetSubsystem<UItemDataSubsystem>();
	if (!Subsystem)
	{
		return;
	}
	
	if (ItemSlot.Num() != ReplicatedSlotIDs.Num())
	{
	 ReplicatedSlotIDs.SetNum(ItemSlot.Num());
	 return;
	}
	
	for (int32 i = 0; i < ReplicatedSlotIDs.Num(); i++)
	{
		const int32 ItemID = ReplicatedSlotIDs[i];
		if (ItemID == -1)
		{
			if (ItemSlot[i])
			{
				ItemSlot[i] = nullptr;
				OnInventorySlotChanged.Broadcast(i, nullptr);
			}
		}
		else
		{
			UItemDataAsset* Data = Subsystem->GetItemDataAsset(ItemID);
			if (!Data)
			{
				continue;
			}

			// 이미 같은 아이템이면 스킵
			if (ItemSlot[i] && ItemSlot[i]->GetItemData() == Data)
			{
				continue;
			}

			UItemInstance* NewItem = NewObject<UItemInstance>(this);
			NewItem->Initialize(Data, GetOwner());
			ItemSlot[i] = NewItem;
			OnInventorySlotChanged.Broadcast(i, NewItem);
		}
	}
}

void UInventoryComponent::ServerRPC_PurchaseItem_Implementation(int32 ItemID)
{
	PurchaseItem(ItemID);
}

void UInventoryComponent::ServerRPC_SellItem_Implementation(int32 SlotIndex)
{
	SellItem(SlotIndex);
}

void UInventoryComponent::ServerRPC_Undo_Implementation()
{
	Undo();
}

void UInventoryComponent::ClientRPC_SyncGold_Implementation(float NewGold)
{
	Gold = NewGold;
	OnGoldChanged.Broadcast(Gold);
}

void UInventoryComponent::OnRep_SlotIDs()
{
	RebuildSlotFromIDs();
}

void UInventoryComponent::OnRep_TrinketID()
{
	if (ReplicatedTrinketID == -1)
	{
		TrinketSlot = nullptr;
		return;
	}
	
	UItemDataSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UItemDataSubsystem>();
	if (!Subsystem) return;

	UItemDataAsset* Data = Subsystem->GetItemDataAsset(ReplicatedTrinketID);
	if (!Data) return;

	UItemInstance* NewInstance = NewObject<UItemInstance>(this);
	NewInstance->Initialize(Data, GetOwner());
	TrinketSlot = NewInstance;
}
