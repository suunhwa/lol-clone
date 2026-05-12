// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/ItemStruct.h"
#include "Item/ItemDataAsset.h"
#include "ItemDataSubsystem.generated.h"

class UItemEffectRegistry;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UItemDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 외부 접근 API
	UItemDataAsset* GetItemDataAsset(int32 ItemID) const;

#pragma region DataTable Accessors
	const FItemBaseRow* GetItemBase(int32 ItemID) const;
	TArray<const FItemStatRow*> GetItemStats(int32 ItemID) const;
	TArray<const FItemEffectRow*> GetItemEffect(const FString& EffectIDR) const;
	const FItemStatTypeRow* GetStatTypeInfo(const FString& StatType) const;
	TMap<int32, TObjectPtr<UItemDataAsset>> GetAllItemDataAssets() const { return ItemAssetMap; }
#pragma endregion
	
private:
	void LoadDataTables();
	void LoadRegistry();
	void LoadItemIcons();
	
	// RowData를 바탕으로 DataAsset 조립
	void BuildItemDataAssets();
	UItemDataAsset* BuildSingleItem(const FItemBaseRow* BaseRow);
	
	// StatType 문자열 → ELolStatType 변환
	bool ResolveStatType(const FString& StatTypeStr, ELolStatType& OutType) const;
	bool ResolveOp(const FString& OpStr, EModifierOp& OutOp) const;
	
private:
	UPROPERTY()
	TMap<int32, TObjectPtr<UItemDataAsset>> ItemAssetMap;
	
	UPROPERTY()
	TObjectPtr<UItemEffectRegistry> EffectRegistry;
	
	UPROPERTY()
	TMap<int32, TObjectPtr<UTexture2D>> IconMap;
	
	UPROPERTY()
	TObjectPtr<UDataTable> DT_ItemBase;
	
	UPROPERTY()
	TObjectPtr<UDataTable> DT_ItemEffect;
	
	UPROPERTY()
	TObjectPtr<UDataTable> DT_ItemStat;
	
	UPROPERTY()
	TObjectPtr<UDataTable> DT_ItemStatType;
};
