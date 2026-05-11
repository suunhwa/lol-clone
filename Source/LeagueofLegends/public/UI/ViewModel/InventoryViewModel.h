// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModelBase.h"
#include "InventoryViewModel.generated.h"

class UItemInstance;
class UInventoryComponent;

USTRUCT()
struct FInventorySlotViewData
{
	GENERATED_BODY()

	int32 SlotIndex = INDEX_NONE;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> Icon;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemSlotUpdated, const FInventorySlotViewData);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventoryGoldChanged, float);
UCLASS()
class LEAGUEOFLEGENDS_API UInventoryViewModel : public UViewModelBase
{
	GENERATED_BODY()
	
public:
	virtual void Initialize() override;
	virtual void Reset() override;
	
	void SetInventoryComponent(UInventoryComponent* InInventoryComp) { InventoryComp = InInventoryComp; }
	
	void HandleInventorySlotChanged(int32 SlotIndex, UItemInstance* ItemInstance);
	void HandleGoldChanged(float NewGold);

public:
	FOnItemSlotUpdated OnItemSlotUpdated;
	FOnInventoryGoldChanged OnGoldChanged;
	
private:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;
};
