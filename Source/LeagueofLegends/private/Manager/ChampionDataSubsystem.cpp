// Fill out your copyright notice in the Description page of Project Settings.

#include "Manager/ChampionDataSubsystem.h"

#include "LeagueofLegends.h"
#include "Characters/Data/ChampionData.h"
#include "Characters/Data/ChampionDataRegistry.h"
#include "Characters/LoLCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UChampionDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 챔피언 DataAsset 레지스트리 로드 → ChampionID 맵 빌드
	// TODO: 실제 에셋 경로로 교체
	UChampionDataRegistry* Registry = Cast<UChampionDataRegistry>(
		StaticLoadObject(UChampionDataRegistry::StaticClass(), nullptr,
		                 TEXT("/Game/Data/DA_ChampionRegistry.DA_ChampionRegistry")));

	if (Registry)
	{
		for (UChampionData* Data : Registry->AllChampions)
		{
			if (Data && !Data->ChampionID.IsNone())
				ChampionDataMap.Add(Data->ChampionID, Data);
		}
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: 챔피언 %d종 로드 완료"), ChampionDataMap.Num());
	}
	else
	{
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: DA_ChampionRegistry 로드 실패. 경로 확인 필요"));
	}

	// 공용 스탯 DataTable 로드
	// TODO: 실제 에셋 경로로 교체
	BaseTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr,
	                                              TEXT("/Game/Data/ChampionDataTable_Base.ChampionDataTable_Base")));
	StatTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr,
	                                              TEXT("/Game/Data/ChampionDataTable_Stat.ChampionDataTable_Stat")));
	GrowthTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr,
	                                                TEXT(
		                                                "/Game/Data/ChampionDataTable_Growth.ChampionDataTable_Growth")));

	if (!BaseTable || !StatTable || !GrowthTable)
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: 스탯 DataTable 로드 실패. 경로 확인 필요"));
}

// DataAsset 조회 
UChampionData* UChampionDataSubsystem::GetChampionData(FName ChampionID) const
{
	TObjectPtr<UChampionData> const* Found = ChampionDataMap.Find(ChampionID);
	if (!Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChampionDataSubsystem: [%s] DataAsset 없음"), *ChampionID.ToString());
		return nullptr;
	}
	return *Found;
}

// Row 조회 
template <typename T>
const T* UChampionDataSubsystem::FindRowByID(UDataTable* Table, const FName& ID)
{
	if (!Table) return nullptr;

	const FString IDStr = ID.ToString();
	for (const FName& RowName : Table->GetRowNames())
	{
		if (RowName.ToString().Contains(IDStr, ESearchCase::IgnoreCase))
			return Table->FindRow<T>(RowName, TEXT(""));
	}
	return nullptr;
}

const FChampionBaseRow* UChampionDataSubsystem::GetBaseRow(FName ChampionID) const
{
	return FindRowByID<FChampionBaseRow>(BaseTable, ChampionID);
}

const FChampionStatRow* UChampionDataSubsystem::GetStatRow(FName ChampionID) const
{
	return FindRowByID<FChampionStatRow>(StatTable, ChampionID);
}

const FChampionGrowthRow* UChampionDataSubsystem::GetGrowthRow(FName ChampionID) const
{
	return FindRowByID<FChampionGrowthRow>(GrowthTable, ChampionID);
}

// 초기화
void UChampionDataSubsystem::ApplyVisuals(ALoLCharacterBase* Target, UChampionData* Data) const
{
	if (!Target || !Data) return;

	if (Data->Mesh)
	{
		Target->GetMesh()->SetSkeletalMesh(Data->Mesh);
		Target->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		Target->GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}

	if (Data->AnimBP)
		Target->GetMesh()->SetAnimInstanceClass(Data->AnimBP);
}

void UChampionDataSubsystem::ApplyStats(ALoLCharacterBase* Target, UChampionData* Data) const
{
	if (!Target || !Data || !Target->StatComp) return;

	const FChampionBaseRow* BaseRow = GetBaseRow(Data->ChampionID);
	const FChampionStatRow* StatRow = GetStatRow(Data->ChampionID);
	const FChampionGrowthRow* GrowthRow = GetGrowthRow(Data->ChampionID);

	if (!BaseRow || !StatRow || !GrowthRow)
	{
		PRINTLOG_SH(TEXT("[%s] ChampionDataSubsystem: Row 못 찾음 — Base:%d Stat:%d Growth:%d"),
		            *Data->ChampionID.ToString(), !!BaseRow, !!StatRow, !!GrowthRow);
		return;
	}

	Target->StatComp->InitStats(*BaseRow, *StatRow, *GrowthRow);

	if (ACharacter* Char = Cast<ACharacter>(Target))
	{
		Char->GetCharacterMovement()->MaxWalkSpeed = Target->StatComp->GetMoveSpeed();
	}

	PRINTLOG_SH(TEXT("[%s] ApplyStats — HP:%.f Mana:%.f AD:%.1f Armor:%.1f MS:%.0f"),
	            *Data->ChampionID.ToString(),
	            Target->StatComp->GetCurrentHP(), Target->StatComp->GetCurrentMana(),
	            Target->StatComp->GetAD(), Target->StatComp->GetArmor(), Target->StatComp->GetMoveSpeed());
}
