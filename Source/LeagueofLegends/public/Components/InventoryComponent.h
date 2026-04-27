// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UItemDataAsset;
class UItemInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LEAGUEOFLEGENDS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
public:
#pragma region Getter Setter
	float GetGold() const { return Gold; }
	void SetGold(float NewGold) { Gold = NewGold; }
#pragma endregion
	
	void AddGold(float Amount) { SetGold(Gold + Amount); }
	void SpendGold(float Amount) { SetGold(Gold - Amount); }
	
public:
	bool PurchaseItem(UItemDataAsset* ItemData);
	void SellItem(int32 SlotIndex);
	void UndoPurchase();
	bool EquipTrinket(UItemDataAsset* TrinketData);

private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float Gold = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Slot")
	int32 MaxSlotCount = 6;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Slot")
	TArray<UItemInstance*> ItemSlot;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Slot")	
	TObjectPtr<UItemInstance> TrinketSlot;
	
	// 구매 내역 스택 (구매 취소용)
	UPROPERTY(VisibleAnywhere, Category = "Inventory|History")
	TArray<UItemDataAsset*> PurchaseHistoryStack;
};

