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
	UChampionDataRegistry* Registry = Cast<UChampionDataRegistry>(
		StaticLoadObject(UChampionDataRegistry::StaticClass(),
		                 nullptr,
		                 TEXT("/Game/Champions/Data/DA_ChampionRegistry.DA_ChampionRegistry")));

	if (Registry)
	{
		for (UChampionData* Data : Registry->AllChampions) 
		{
			if (Data && !Data->ChampionID.IsNone())
			{
				ChampionDataMap.Add(Data->ChampionID, Data);
			}
		}
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: 챔피언 %d종 로드 완료"), ChampionDataMap.Num());
	}
	else
	{
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: DA_ChampionRegistry 로드 실패. 경로 확인 필요"));
	}

	// 공용 스탯 DataTable 로드
	BaseTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                              nullptr,
	                                              TEXT(
		                                              "/Game/DataTables/ChampionStatDataTable_ChampionBase.ChampionStatDataTable_ChampionBase")));
	StatTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                              nullptr,
	                                              TEXT(
		                                              "/Game/DataTables/ChampionStatDataTable_ChampionStatValues.ChampionStatDataTable_ChampionStatValues")));
	GrowthTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                                nullptr,
	                                                TEXT(
		                                                "/Game/DataTables/ChampionStatDataTable_ChampionGrowth.ChampionStatDataTable_ChampionGrowth")));

	if (!BaseTable || !StatTable || !GrowthTable)
	{
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: 스탯 DataTable 로드 실패. 경로 확인 필요"));
	}

	MasterSkillTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                                     nullptr,
	                                                     TEXT(
		                                                     "/Game/DataTables/ChampionSkillDataTable_MasterSkillCore.ChampionSkillDataTable_MasterSkillCore")));
	DetailSkillStatsTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                                          nullptr,
	                                                          TEXT(
		                                                          "/Game/DataTables/ChampionSkillDataTable_DetailSkillStats.ChampionSkillDataTable_DetailSkillStats")));
	MechanicsTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                                   nullptr,
	                                                   TEXT(
		                                                   "/Game/DataTables/ChampionSkillDataTable_SkillMechanics.ChampionSkillDataTable_SkillMechanics")));

	if (!MasterSkillTable || !DetailSkillStatsTable || !MechanicsTable)
	{
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: 스킬 DataTable 로드 실패. 경로 확인 필요"));
	}

	PlayerLevelExpTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                                        nullptr,
	                                                        TEXT(
		                                                        "/Script/Engine.DataTable'/Game/DataTables/PlayerLevelExp.PlayerLevelExp'")));
	
	UnitRewardTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
															nullptr,
															TEXT(
																"/Script/Engine.DataTable'/Game/DataTables/UnitRewardExp.UnitRewardExp'")));
	
	ChampionKillTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
															nullptr,
															TEXT(
																"/Script/Engine.DataTable'/Game/DataTables/ChampionKillExp.ChampionKillExp'")));
	
	if (!PlayerLevelExpTable || !UnitRewardTable || !ChampionKillTable)
	{
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: 경험치 DataTable 로드 실패. 경로 확인 필요"));
	}

	RespawnTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(),
	                                                 nullptr,
	                                                 TEXT("/Game/DataTables/ChampionStatDataTable_ChampionRespawn.ChampionStatDataTable_ChampionRespawn")));

	if (!RespawnTable)
	{
		PRINTLOG_SH(TEXT("ChampionDataSubsystem: RespawnTable 로드 실패. 경로 확인 필요"));
	}
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

TArray<UChampionData*> UChampionDataSubsystem::GetAllChampions() const
{
	TArray<UChampionData*> Result;
	for (auto& Pair : ChampionDataMap)
	{
		if (Pair.Value)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// Row 조회
template <typename T>
const T* UChampionDataSubsystem::FindRowByID(UDataTable* Table, const FName& ID)
{
	if (!Table) { return nullptr; }

	const FString IDStr = ID.ToString();
	for (const FName& RowName : Table->GetRowNames())
	{
		if (RowName.ToString().Contains(IDStr, ESearchCase::IgnoreCase))
		{
			return Table->FindRow<T>(RowName, TEXT(""));
		}
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

const FDetailSkillStatsRow* UChampionDataSubsystem::GetSkillStats(FName ChampionID,
                                                                  const FString& SkillKey,
                                                                  int32 Step) const
{
	if (!DetailSkillStatsTable) { return nullptr; }

	const FString SearchStr = ChampionID.ToString() + TEXT("_") + SkillKey;

	for (const FName& RowName : DetailSkillStatsTable->GetRowNames())
	{
		const FDetailSkillStatsRow* Row = DetailSkillStatsTable->FindRow<FDetailSkillStatsRow>(RowName, TEXT(""));
		if (!Row) { continue; }

		if (Row->Skill_ID.Contains(SearchStr, ESearchCase::IgnoreCase) && Row->Step == Step)
		{
			return Row;
		}
	}
	return nullptr;
}

const FSkillMechanicsRow* UChampionDataSubsystem::GetSkillMechanics(FName ChampionID, const FString& SkillKey) const
{
	if (!MasterSkillTable || !MechanicsTable) { return nullptr; }

	const FString SearchStr = ChampionID.ToString() + TEXT("_") + SkillKey;

	// 1단계: MasterSkill에서 Effect_Tag 조회
	FString EffectTag;
	for (const FName& RowName : MasterSkillTable->GetRowNames())
	{
		const FMasterSkillCoreRow* Row = MasterSkillTable->FindRow<FMasterSkillCoreRow>(RowName, TEXT(""));
		if (!Row) { continue; }

		if (Row->Skill_ID.Contains(SearchStr, ESearchCase::IgnoreCase))
		{
			EffectTag = Row->Effect_Tag;
			break;
		}
	}

	if (EffectTag.IsEmpty()) { return nullptr; }

	// 2단계: Mechanics 테이블에서 Effect_Tag로 Row 조회
	return MechanicsTable->FindRow<FSkillMechanicsRow>(FName(*EffectTag), TEXT(""));
}

// 경험치 테이블 조회
const FPlayerLevelExpRow* UChampionDataSubsystem::GetPlayerLevelExpRow(int32 Level) const
{
	if (!PlayerLevelExpTable) { return nullptr; }
	// Row Name은 레벨 숫자 문자열 ("1", "2", ..., "17")
	return PlayerLevelExpTable->FindRow<FPlayerLevelExpRow>(FName(*FString::FromInt(Level)), TEXT(""));
}

const FUnitRewardExpRow* UChampionDataSubsystem::GetUnitRewardRow(FName UnitType) const
{
	if (!UnitRewardTable) { return nullptr; }
	return UnitRewardTable->FindRow<FUnitRewardExpRow>(UnitType, TEXT(""));
}

const FChampionKillExpRow* UChampionDataSubsystem::GetChampionKillExpRow(int32 EnemyLevel) const
{
	if (!ChampionKillTable) { return nullptr; }
	return ChampionKillTable->FindRow<FChampionKillExpRow>(FName(*FString::FromInt(EnemyLevel)), TEXT(""));
}

const FChampionRespawnRow* UChampionDataSubsystem::GetRespawnRow(int32 Level) const
{
	if (!RespawnTable) { return nullptr; }
	return RespawnTable->FindRow<FChampionRespawnRow>(FName(*FString::FromInt(Level)), TEXT(""));
}

// 초기화
void UChampionDataSubsystem::ApplyVisuals(ALoLCharacterBase* Target, UChampionData* Data) const
{
	if (!Target || !Data) { return; }

	if (Data->Mesh)
	{
		Target->GetMesh()->SetSkeletalMesh(Data->Mesh);
		Target->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		Target->GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}

	if (Data->AnimBP)
	{
		Target->GetMesh()->SetAnimInstanceClass(Data->AnimBP);
	}
}

void UChampionDataSubsystem::ApplyStats(ALoLCharacterBase* Target, UChampionData* Data) const
{
	if (!Target || !Data || !Target->StatComp) { return; }

	const FChampionBaseRow* BaseRow = GetBaseRow(Data->ChampionID);
	const FChampionStatRow* StatRow = GetStatRow(Data->ChampionID);
	const FChampionGrowthRow* GrowthRow = GetGrowthRow(Data->ChampionID);

	if (!BaseRow || !StatRow || !GrowthRow)
	{
		PRINTLOG_SH(TEXT("[%s] ChampionDataSubsystem: Row 못 찾음 — Base:%d Stat:%d Growth:%d"),
		            *Data->ChampionID.ToString(),
		            !!BaseRow,
		            !!StatRow,
		            !!GrowthRow);
		return;
	}

	Target->StatComp->InitStats(*BaseRow, *StatRow, *GrowthRow);

	if (ACharacter* Char = Cast<ACharacter>(Target))
	{
		Char->GetCharacterMovement()->MaxWalkSpeed = Target->StatComp->GetMoveSpeed();
	}

	PRINTLOG_SH(TEXT("[%s] ApplyStats — HP:%.f Mana:%.f AD:%.1f Armor:%.1f MS:%.0f"),
	            *Data->ChampionID.ToString(),
	            Target->StatComp->GetCurrentHP(),
	            Target->StatComp->GetCurrentMana(),
	            Target->StatComp->GetAD(),
	            Target->StatComp->GetArmor(),
	            Target->StatComp->GetMoveSpeed());
}



