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
	void ClearHistory() { PurchaseHistoryStack.Empty(); }
	bool EquipTrinket(UItemDataAsset* TrinketData);
	
	FOnInventorySlotChanged OnInventorySlotChanged;
	FOnGoldChanged OnGoldChanged;

private:
	int GetEmptySlotIndex() const;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float Gold = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Slot")
	int32 MaxSlotCount = 6;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Slot")
	TArray<TObjectPtr<UItemInstance>> ItemSlot;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Slot")	
	TObjectPtr<UItemInstance> TrinketSlot;
	
	// 구매 내역 스택 (구매 취소용)
	UPROPERTY(VisibleAnywhere, Category = "Inventory|History")
	TArray<FPurchaseRecord> PurchaseHistoryStack;
};

