#include "Manager/MinionDataSubsystem.h"

#include "LeagueofLegends.h"

void UMinionDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 에셋 경로 직접 지정, 자동 로드
	BaseTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/Data/MinionStatDataTable_MinionBase.MinionStatDataTable_MinionBase")));
	GrowthTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/Data/MinionStatDataTable_MinionGrowth.MinionStatDataTable_MinionGrowth")));
	WaveTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/Data/MinionStatDataTable_MinionWave.MinionStatDataTable_MinionWave")));
	if (!BaseTable || !GrowthTable)
	{
		UE_LOG(LogTemp, Error, TEXT("서브시스템: 데이터 테이블 로드 실패! 경로를 확인하세요."));
	}
	else
	{
		PRINTLOG_HJ(TEXT("서브시스템: 데이터 테이블 로드 성공!"));
	}
	
}

FMinionBaseRow* UMinionDataSubsystem::GetBaseRowByID(int32 TargetID)
{
	if (!BaseTable) return nullptr;

	// 모든 행을 가져와서 루프를 돌며 ID를 비교합니다.
	TArray<FMinionBaseRow*> AllRows;
	BaseTable->GetAllRows<FMinionBaseRow>(TEXT(""), AllRows);

	for (auto Row : AllRows)
	{
		if (Row && Row->MinionID == TargetID)
		{
			return Row;
		}
	}
	return nullptr;
}

FMinionGrowthRow* UMinionDataSubsystem::GetGrowthRowByID(int32 TargetID)
{
	if (!GrowthTable) return nullptr;

	TArray<FMinionGrowthRow*> AllRows;
	GrowthTable->GetAllRows<FMinionGrowthRow>(TEXT(""), AllRows);

	for (auto Row : AllRows)
	{
		// GrowthStruct의 변수명인 Minion_ID와 비교
		if (Row && Row->Minion_ID == TargetID)
		{
			return Row;
		}
	}
	return nullptr;
}

FMinionWaveRow* UMinionDataSubsystem::GetWaveRow(FName RowName)
{
	return WaveTable ? WaveTable->FindRow<FMinionWaveRow>(RowName, TEXT("")) : nullptr;
}