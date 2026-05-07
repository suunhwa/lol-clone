// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Item/ItemDataAsset.h"
#include "ItemInstance.generated.h"

/**
 * 아이템 하나당 1개의 Instance가 존재한다. (예: 체력 물약 3개를 들고 있으면, ItemDataAsset은 하나지만 ItemInstance는 3개)
 */
UCLASS()
class LEAGUEOFLEGENDS_API UItemInstance : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(UItemDataAsset* InDataAsset, AActor* InOwner);
	
	void OnEquipped();
	void OnUnequipped();
	
	UItemDataAsset* GetItemData() const { return ItemDataAsset; }
	
private:
	UPROPERTY()
	TObjectPtr<UItemDataAsset> ItemDataAsset;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerChampion;
	
	TArray<FStatModifierHandle> RegisteredHandles;
	
	UPROPERTY()
	TArray<TObjectPtr<UItemPassiveEffectBase>> ActivePassives;
};
