#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/ObjectStruct.h"
#include "ObjectDataSubsystem.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API UObjectDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 포탑 ID 하나로 세 가지 데이터를 한 번에 가져오는 편의 함수
	bool GetAllTowerData(int32 InID, FObjectBaseRow& OutBase, FObjectRewardRow& OutReward, FObjectMechanicsRow& OutMech);

private:
	// 데이터 테이블 포인터
	UPROPERTY() TObjectPtr<UDataTable> BaseTable;
	UPROPERTY() TObjectPtr<UDataTable> RewardTable;
	UPROPERTY() TObjectPtr<UDataTable> MechanicsTable;

	// 빠른 검색을 위한 캐싱용 Map
	TMap<int32, FObjectBaseRow*> BaseMap;
	TMap<int32, FObjectRewardRow*> RewardMap;
	TMap<int32, FObjectMechanicsRow*> MechMap;
};