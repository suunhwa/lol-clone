// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemDataAsset;
class UItemInstance;

USTRUCT(BlueprintType)
struct FPurchaseRecord
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 ActionType{0}; // 0: 구매, 1: 판매
	
	UPROPERTY()
	int32 SlotIndex{INDEX_NONE}; // 판매할 때만 채워지는 값
	
	UPROPERTY()
	TObjectPtr<UItemInstance> ItemInstance = nullptr;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotChanged, int32, UItemInstance*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, float);
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
#pragma region Getter Setter
	float GetGold() const { return Gold; }
	
	UFUNCTION(BlueprintCallable)
	void SetGold(float NewGold);
	
	UFUNCTION(BlueprintCallable)
	void AddGold(float Amount) { SetGold(Gold + Amount); }
	
	UFUNCTION(BlueprintCallable)
	void SpendGold(float Amount) { SetGold(Gold - Amount); }
	
	UItemInstance* GetItemAtSlot(int32 SlotIndex) const;
	UItemInstance* GetTrinket() const { return TrinketSlot; }
	
	bool IsHistoryEmpty() const { return PurchaseHistoryStack.Num() == 0; }
#pragma endregion
	
	bool PurchaseItem(int32 ItemID);
	void SellItem(int32 SlotIndex);
	void Undo();
	bool EquipTrinket(UItemDataAsset* TrinketData);	
	
	FOnInventorySlotChanged OnInventorySlotChanged;
	FOnGoldChanged OnGoldChanged;

private:
	// -------- 내부 유틸 --------
	int32 GetEmptySlotIndex() const;

	// ItemSlot → ReplicatedSlotIDs 동기화 후 복제 트리거
	void SyncSlotIDsToClients();

	// 클라이언트에서 ItemID로 UItemInstance 재구성
	void RebuildSlotFromIDs();
	
	// -------- Server RPC --------
	UFUNCTION(Server, Reliable)
	void ServerRPC_PurchaseItem(int32 ItemID);
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_SellItem(int32 SlotIndex);
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_Undo();
	
	// -------- Client RPC --------
	UFUNCTION(Client, Reliable)
	void ClientRPC_SyncGold(float NewGold);
	
private:	
	// -------- Replication --------
	// SlotID 배열: UItemInstance 직접 복제 대신 ItemID(int32)로 복제
	// -1 = 빈 슬롯 / OwnerOnly: 내 인벤토리는 나만 받음
	UPROPERTY(ReplicatedUsing = OnRep_SlotIDs, VisibleAnywhere, Category = "Inventory|Slot")
	TArray<int32> ReplicatedSlotIDs;

	UPROPERTY(ReplicatedUsing = OnRep_TrinketID, VisibleAnywhere, Category = "Inventory|Slot")
	int32 ReplicatedTrinketID = -1;

	// -------- RepNotify --------
	UFUNCTION()
	void OnRep_SlotIDs();

	UFUNCTION()
	void OnRep_TrinketID();
	
private:
	// -------- 서버 전용 상태 --------
	// Gold: 서버에서만 권위 있게 관리, ClientRPC로 OwningClient에 통보
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float Gold = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Slot")
	int32 MaxSlotCount = 6;
	
	// 실제 인스턴스 배열 — 서버에서 로직 처리용, 클라이언트는 OnRep 이후 재구성
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Slot")
	TArray<TObjectPtr<UItemInstance>> ItemSlot;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Slot")	
	TObjectPtr<UItemInstance> TrinketSlot;
	
	// 구매 내역 스택 (구매 취소용) - 서버 전용, 복제 불필요
	UPROPERTY(VisibleAnywhere, Category = "Inventory|History")
	TArray<FPurchaseRecord> PurchaseHistoryStack;
};

