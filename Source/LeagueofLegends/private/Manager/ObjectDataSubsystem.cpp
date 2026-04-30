#include "Manager/ObjectDataSubsystem.h"

void UObjectDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 1. 테이블 로드 (경로는 실제 프로젝트 위치에 맞게 수정하세요)
	BaseTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/Data/ObjectStatDataTable_ObjectBase.ObjectStatDataTable_ObjectBase")));
	RewardTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/Data/ObjectStatDataTable_ObjectReward.ObjectStatDataTable_ObjectReward")));
	MechanicsTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/Data/ObjectStatDataTable_ObjectMechanics.ObjectStatDataTable_ObjectMechanics")));

	// 2. TMap에 데이터 캐싱 (런타임 속도 향상)
	if (BaseTable) {
		TArray<FObjectBaseRow*> Rows;
		BaseTable->GetAllRows<FObjectBaseRow>(TEXT(""), Rows);
		for (auto Row : Rows) BaseMap.Add(Row->Object_ID, Row);
	}
	if (RewardTable) {
		TArray<FObjectRewardRow*> Rows;
		RewardTable->GetAllRows<FObjectRewardRow>(TEXT(""), Rows);
		for (auto Row : Rows) RewardMap.Add(Row->Object_ID, Row);
	}
	if (MechanicsTable) {
		TArray<FObjectMechanicsRow*> Rows;
		MechanicsTable->GetAllRows<FObjectMechanicsRow>(TEXT(""), Rows);
		for (auto Row : Rows) MechMap.Add(Row->Object_ID, Row);
	}
}

bool UObjectDataSubsystem::GetAllTowerData(int32 InID, FObjectBaseRow& OutBase, FObjectRewardRow& OutReward, FObjectMechanicsRow& OutMech)
{
	if (BaseMap.Contains(InID) && RewardMap.Contains(InID) && MechMap.Contains(InID))
	{
		OutBase = *BaseMap[InID];
		OutReward = *RewardMap[InID];
		OutMech = *MechMap[InID];
		return true;
	}
	return false;
}