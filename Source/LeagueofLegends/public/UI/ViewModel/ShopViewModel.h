// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModelBase.h"
#include "ShopViewModel.generated.h"

enum class ELolStatType : uint8;
class UItemDataAsset;
class UInventoryComponent;
class UItemDataSubsystem;

USTRUCT()
struct FItemProfileViewData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	FText Name;
	
	UPROPERTY()
	int32 Price = 0;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> Icon;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnShopItemsBuilt, const TArray<FItemProfileViewData>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGoldUpdated, int32);
UCLASS()
class LEAGUEOFLEGENDS_API UShopViewModel : public UViewModelBase
{
	GENERATED_BODY()
	
public:
	virtual void Initialize() override;
	virtual void Reset() override;
	
	// const TArray<FItemProfileViewData>& GetShopItemViewData() const { return CachedViewData; }
	
	UItemDataAsset* GetItemDataAsset(int32 ItemID) const;
	UTexture2D* GetItemStatIcon(ELolStatType StatType) const;
	FString GetStatNameKR(ELolStatType StatType) const;

	void SetInventoryComponent(UInventoryComponent* InInventoryComp) { InventoryComp = InInventoryComp; }
	void SetItemDataSubsystem(UItemDataSubsystem* InItemSubsystem) { ItemSubsystem = InItemSubsystem; }
	
	// 사용자 입력(ShopWidget에서 호출)
	void RequestPurchase(int32 ItemID);
	void RequestSell(int32 SlotIndex);
	void RequestUndo();

private:
	void BuildViewData();
	void HandleGoldChanged(float NewGold);

public:
	FOnShopItemsBuilt OnShopItemsBuilt;
	FOnGoldUpdated OnGoldUpdated;
	
private:
	UPROPERTY()
	TObjectPtr<UItemDataSubsystem> ItemSubsystem;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;

	// TArray<FItemProfileViewData> CachedViewData;
};
