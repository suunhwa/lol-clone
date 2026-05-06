// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/ItemDataSubsystem.h"
#include "Item/ItemEffectRegistry.h"
#include "LeagueofLegends.h"
#include "Item/ItemPassiveEffectBase.h"
#include "Item/ItemSettings.h"

void UItemDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	LoadDataTables();
	LoadRegistry();
	BuildItemDataAssets();
}

UItemDataAsset* UItemDataSubsystem::GetItemDataAsset(int32 ItemID) const
{
	if (const TObjectPtr<UItemDataAsset>* Found = ItemAssetMap.Find(ItemID))
	{
		return Found->Get();
	}
	else
	{
		PRINTLOG_TK(TEXT("ItemDataAsset not found for ItemID: %d"), ItemID);
		return nullptr;
	}
}

const FItemBaseRow* UItemDataSubsystem::GetItemBase(int32 ItemID) const
{
	if (!DT_ItemBase)
	{
		PRINTLOG_TK(TEXT("Invalid DataTable: DT_ItemBase"));
		return nullptr;
	}
	
	return DT_ItemBase->FindRow<FItemBaseRow>(FName(*FString::FromInt(ItemID)), TEXT("GetItemBase"));
}

TArray<const FItemStatRow*> UItemDataSubsystem::GetItemStats(int32 ItemID) const
{
	TArray<const FItemStatRow*> Result;
	if (!DT_ItemStat)
	{
		PRINTLOG_TK(TEXT("Invalid DataTable: DT_ItemStat"));
		return Result;
	}
	
	int32 Index = 1;
	while (true)
	{
		// 1001_01, 1001_02 ... 형식
		FString RowName = FString::Printf(TEXT("%d_%02d"), ItemID, Index);
		const FItemStatRow* Row = DT_ItemStat->FindRow<FItemStatRow>(
			FName(*RowName), TEXT("GetItemStats"), false);

		if (!Row)
		{
			break;
		}
		
		Result.Add(Row);
		Index++;
	}

	return Result;
}

TArray<const FItemEffectRow*> UItemDataSubsystem::GetItemEffect(const FString& EffectIDR) const
{
	TArray<const FItemEffectRow*> Result;
	if (!DT_ItemEffect || EffectIDR.IsEmpty())
	{
		PRINTLOG_TK(TEXT("Invalid DataTable or EffectIDR: %s"), *EffectIDR);
		return Result;
	}
	
	// -1이면 이펙트 없음
	if (EffectIDR.TrimStartAndEnd() == TEXT("-1"))
	{
		return Result;
	}
	
	TArray<FString> IDs;
	EffectIDR.ParseIntoArray(IDs, TEXT(","), true);

	for (FString& ID : IDs)
	{
		ID.TrimStartAndEndInline();
		const FItemEffectRow* Row = DT_ItemEffect->FindRow<FItemEffectRow>(
			FName(*ID), TEXT("GetItemEffects"), false);

		if (Row)
		{
			Result.Add(Row);
		}
		else
		{
			PRINTLOG_TK(TEXT("Effect Row not found: %s"), *ID);
		}
	}
	
	return Result;
}

const FItemStatTypeRow* UItemDataSubsystem::GetStatTypeInfo(const FString& StatType) const
{
	if (!DT_ItemStatType)
	{
		PRINTLOG_TK(TEXT("Invalid DataTable: DT_ItemStatType"));
		return nullptr;
	}

	return DT_ItemStatType->FindRow<FItemStatTypeRow>(
		FName(*StatType), TEXT("GetStatTypeInfo"), false);
}

void UItemDataSubsystem::LoadDataTables()
{   
	DT_ItemBase = LoadObject<UDataTable>(nullptr,
		TEXT("/Script/Engine.DataTable'/Game/Data/ItemDataTable_ItemBase.ItemDataTable_ItemBase'"));

	DT_ItemEffect = LoadObject<UDataTable>(nullptr,
		TEXT("/Script/Engine.DataTable'/Game/Data/ItemDataTable_ItemEffect.ItemDataTable_ItemEffect'"));

	DT_ItemStat = LoadObject<UDataTable>(nullptr,
		TEXT("/Script/Engine.DataTable'/Game/Data/ItemDataTable_ItemStat.ItemDataTable_ItemStat'"));

	DT_ItemStatType = LoadObject<UDataTable>(nullptr,
		TEXT("/Script/Engine.DataTable'/Game/Data/ItemDataTable_ItemStatType.ItemDataTable_ItemStatType'"));

	// 로드 검증
	PRINTLOG_TK(TEXT("DT_ItemBase     : %s"), DT_ItemBase     ? TEXT("OK") : TEXT("FAILED"));
	PRINTLOG_TK(TEXT("DT_ItemEffect   : %s"), DT_ItemEffect   ? TEXT("OK") : TEXT("FAILED"));
	PRINTLOG_TK(TEXT("DT_ItemStat     : %s"), DT_ItemStat     ? TEXT("OK") : TEXT("FAILED"));
	PRINTLOG_TK(TEXT("DT_ItemStatType : %s"), DT_ItemStatType ? TEXT("OK") : TEXT("FAILED"));
}

void UItemDataSubsystem::LoadRegistry()
{
	const UItemSettings* Settings = GetDefault<UItemSettings>();
	if (Settings && Settings->ItemEffectRegistry.IsValid())
	{
		UItemEffectRegistry* Registry = Settings->ItemEffectRegistry.LoadSynchronous();
		if (Registry)
		{
			PRINTLOG_TK(TEXT("ItemEffectRegistry loaded successfully"));
			// 필요한 경우 Registry 데이터를 캐싱하거나 초기화할 수 있음
			EffectRegistry = Registry;
		}
		else
		{
			PRINTLOG_TK(TEXT("Failed to load ItemEffectRegistry"));
		}
	}
	else
	{
		PRINTLOG_TK(TEXT("Invalid ItemSettings or ItemEffectRegistry reference"));
	}
}

void UItemDataSubsystem::BuildItemDataAssets()
{
	if (!DT_ItemBase)
	{
		PRINTLOG_TK(TEXT("Cannot build ItemDataAssets: DT_ItemBase is null"));
		return;
	}
	
	TArray<FItemBaseRow*> AllBaseRows;
	DT_ItemBase->GetAllRows<FItemBaseRow>(TEXT("BuildItemDataAssets"), AllBaseRows);
	for (FItemBaseRow* BaseRow : AllBaseRows)
	{
		if (!BaseRow) continue;
		
		UItemDataAsset* NewAsset = BuildSingleItem(BaseRow);
		if (NewAsset)
		{
			ItemAssetMap.Add(NewAsset->ItemID, NewAsset);
		}
	}

	PRINTLOG_TK(TEXT("ItemDataAsset 빌드 완료: %d개"), ItemAssetMap.Num());
}

UItemDataAsset* UItemDataSubsystem::BuildSingleItem(const FItemBaseRow* BaseRow)
{
	UItemDataAsset* Asset = NewObject<UItemDataAsset>(this);
	
	// Base 복사
	Asset->ItemID       = BaseRow->Item_ID;
	Asset->NameKR       = BaseRow->Name_KR;
	Asset->NameEN       = BaseRow->Name_EN;
	Asset->Price        = BaseRow->Price;
	Asset->RefundPrice  = BaseRow->RefundPrice;
	Asset->bIsActive    = BaseRow->Is_Active;
	Asset->bIsConsumable = BaseRow->Is_Consumable;
	Asset->Cooldown     = BaseRow->Cooldown;
	Asset->MaxStack     = BaseRow->Max_Stack;
	
	// Stat 조립
	TArray<const FItemStatRow*> StatRows = GetItemStats(Asset->ItemID);
	for (const FItemStatRow* StatRow : StatRows)
	{
		FStatModifier StatData;
		// StatType
		if (!ResolveStatType(StatRow->Stat_Type_R, StatData.StatType))
		{
			PRINTLOG_TK(TEXT("Failed to resolve StatType for ItemID %d: %s"), Asset->ItemID, *StatRow->Stat_Type_R);
			continue;
		}
		// Op
		const FItemStatTypeRow* TypeInfo = GetStatTypeInfo(StatRow->Stat_Type_R);
		if (!TypeInfo)
		{
			PRINTLOG_TK(TEXT("StatTypeInfo not found for: %s"), *StatRow->Stat_Type_R);
			continue;
		}

		if (!ResolveOp(TypeInfo->Op, StatData.Op))
		{
			continue;
		}
		// Value
		StatData.Value = StatRow->Stat_Value;
		Asset->Stats.Add(StatData);
	}
	
	// Effect 조립
	TArray<const FItemEffectRow*> EffectRows = GetItemEffect(BaseRow->Effect_ID_R);
	for (const FItemEffectRow* EffectRow : EffectRows)
	{
		FItemPassiveEffectData EffectData;
		EffectData.EffectName = EffectRow->Effect_Name;
		EffectData.Value01 = EffectRow->Value01;
		EffectData.Value02 = EffectRow->Value02;
		EffectData.Description = EffectRow->Description;
		
		// Registry에서 클래스 바인딩
		if (EffectRegistry)
		{
			EffectData.PassiveClass = EffectRegistry->FindPassiveClass(EffectData.EffectName);
			if (!EffectData.PassiveClass)
			{
				PRINTLOG_TK(TEXT("PassiveClass not found for: %s"), *EffectData.EffectName.ToString());
			}
		}
		else
		{
			PRINTLOG_TK(TEXT("EffectRegistry is null — skipping class binding"));
		}


		Asset->Effects.Add(EffectData);
	}
	
	return Asset;
}

bool UItemDataSubsystem::ResolveStatType(const FString& StatTypeStr, ELolStatType& OutType) const
{
	static const TMap<FString, ELolStatType> StatTypeMap =
	{
		{ TEXT("HP"),              ELolStatType::HP             },
		{ TEXT("HPRegen"),         ELolStatType::HPRegen        },
		{ TEXT("HPRegenPercent"),  ELolStatType::HPRegenPercent },
		{ TEXT("MP"),              ELolStatType::MP             },
		{ TEXT("MPRegen"),         ELolStatType::MPRegen        },
		{ TEXT("MPRegenPercent"),  ELolStatType::MPRegenPercent },
		{ TEXT("AD"),              ELolStatType::AD             },
		{ TEXT("ASRatio"),         ELolStatType::ASRatio        },
		{ TEXT("Crit"),            ELolStatType::Crit           },
		{ TEXT("ADPenFlat"),       ELolStatType::ADPenFlat      },
		{ TEXT("ADPenRatio"),      ELolStatType::ADPenRatio     },
		{ TEXT("AP"),              ELolStatType::AP             },
		{ TEXT("AH"),              ELolStatType::AH             },
		{ TEXT("APPenFlat"),       ELolStatType::APPenFlat      },
		{ TEXT("APPenRatio"),      ELolStatType::APPenRatio     },
		{ TEXT("Armor"),           ELolStatType::Armor          },
		{ TEXT("MR"),              ELolStatType::MR             },
		{ TEXT("Tenacity"),        ELolStatType::Tenacity       },
		{ TEXT("MS"),              ELolStatType::MS             },
		{ TEXT("MSPercent"),       ELolStatType::MSPercent      },
		{ TEXT("LS"),              ELolStatType::LS             },
		{ TEXT("Omnivamp"),        ELolStatType::Omnivamp       },
		{ TEXT("HealShieldPower"), ELolStatType::HealShieldPower},
		{ TEXT("Gold"),            ELolStatType::Gold           },
	};
	
	if (const ELolStatType* Found = StatTypeMap.Find(StatTypeStr))
	{
		OutType = *Found;
		return true;
	}
	else
	{
		PRINTLOG_TK(TEXT("Unknown StatType string: %s"), *StatTypeStr);
		return false;
	}
}

bool UItemDataSubsystem::ResolveOp(const FString& OpStr, EModifierOp& OutOp) const
{
	if (OpStr == TEXT("Add"))
	{
		OutOp = EModifierOp::Add;
		return true;
	}
	else if (OpStr == TEXT("Mul"))
	{
		OutOp = EModifierOp::Mul;
		return true;
	}

	PRINTLOG_TK(TEXT("Unknown Op: %s"), *OpStr);
	return false;
}





























