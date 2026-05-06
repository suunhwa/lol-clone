// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/ChampionStatStruct.h"
#include "Struct/ChampionSkillStruct.h"
#include "ChampionDataSubsystem.generated.h"

class ALoLCharacterBase;
class UChampionData;

UCLASS()
class LEAGUEOFLEGENDS_API UChampionDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ChampionID로 DataAsset 조회 (캐릭터 선택 시 사용)
	UChampionData* GetChampionData(FName ChampionID) const;

	// 챔피언 비주얼 적용 (메시, ABP). 서버/클라이언트 모두 호출 가능
	void ApplyVisuals(ALoLCharacterBase* Target, UChampionData* Data) const;

	// 챔피언 스탯 초기화. 서버 전용
	void ApplyStats(ALoLCharacterBase* Target, UChampionData* Data) const;

	// --- 챔피언 스탯 Row 조회 ---
	const FChampionBaseRow*   GetBaseRow(FName ChampionID)   const;
	const FChampionStatRow*   GetStatRow(FName ChampionID)   const;
	const FChampionGrowthRow* GetGrowthRow(FName ChampionID) const;

	// --- 스킬 데이터 조회 ---
	// SkillKey: "Q", "W", "E", "R"  /  Step: 스킬 랭크 (1~5)
	const FDetailSkillStatsRow* GetSkillStats(FName ChampionID, const FString& SkillKey, int32 Step) const;
	const FSkillMechanicsRow*   GetSkillMechanics(FName ChampionID, const FString& SkillKey) const;

private:
	template <typename T>
	static const T* FindRowByID(UDataTable* Table, const FName& ID);

	// ChampionID → DataAsset 맵 (레지스트리에서 빌드)
	UPROPERTY()
	TMap<FName, TObjectPtr<UChampionData>> ChampionDataMap;

	// 챔피언 스탯 DataTable
	UPROPERTY()
	TObjectPtr<UDataTable> BaseTable;

	UPROPERTY()
	TObjectPtr<UDataTable> StatTable;

	UPROPERTY()
	TObjectPtr<UDataTable> GrowthTable;

	// 스킬 DataTable
	UPROPERTY()
	TObjectPtr<UDataTable> MasterSkillTable;

	UPROPERTY()
	TObjectPtr<UDataTable> DetailSkillStatsTable;

	UPROPERTY()
	TObjectPtr<UDataTable> MechanicsTable;
};
