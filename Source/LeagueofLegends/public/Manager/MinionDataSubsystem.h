#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/MinionStruct.h" 
#include "MinionDataSubsystem.generated.h"

UCLASS() 
class LEAGUEOFLEGENDS_API UMinionDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	
	// 서브시스템이 생성될 때 실행 (테이블 로드 가능)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ID(int32)를 기준으로 검색하는 함수들
	FMinionBaseRow* GetBaseRowByID(int32 TargetID);
	FMinionGrowthRow* GetGrowthRowByID(int32 TargetID);
	FMinionWaveRow* GetWaveRow(FName RowName);
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> BaseTable;

	UPROPERTY()
	TObjectPtr<UDataTable> GrowthTable;

	UPROPERTY()
	TObjectPtr<UDataTable> WaveTable;
};