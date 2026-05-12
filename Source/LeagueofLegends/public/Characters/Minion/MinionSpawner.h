#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/RiftTypes.h"      
#include "Struct/MinionStruct.h" 
#include "MinionSpawner.generated.h"

class ALoLMinion;

UCLASS()
class LEAGUEOFLEGENDS_API AMinionSpawner : public AActor
{
	GENERATED_BODY()

public:
	AMinionSpawner();

protected:
	virtual void BeginPlay() override;

	// CSV의 ID(int32)와 소환할 블루프린트 클래스를 에디터에서 매핑
	UPROPERTY(EditAnywhere, Category = "Spawner|Classes")
	TMap<int32, TSubclassOf<ALoLMinion>> MinionClassMap;
	
	// --- 에디터 설정 변수 ---
	UPROPERTY(EditAnywhere, Category = "Spawner|Data")
	UDataTable* WaveDataTable;

	UPROPERTY(EditAnywhere, Category = "Spawner|Settings")
	ETeam SpawnerTeam;

	UPROPERTY(EditAnywhere, Category = "Spawner|Settings")
	ELane SpawnerLane;

	// --- 내부 상태 관리 ---
	int32 CurrentWaveCount = 0;
	FTimerHandle WaveTimerHandle;

	// --- 핵심 함수 ---
	void CheckAndSpawnWave();
	void ExecuteSpawnSequence(const FString& IDListString);
	TArray<int32> ParseIDList(const FString& IDListString);

	// 에디터에서 직접 좌표를 찍을 수 있는 배열
	UPROPERTY(EditAnywhere, Category = "Spawner|Settings", meta = (MakeEditWidget = true))
	TArray<FVector> Waypoints;
	
public:
	// 외부(억제기 파괴 시)에서 호출할 변수
	UPROPERTY(BlueprintReadWrite, Category = "Spawner|State")
	bool bIsInhibitorDestroyed = false;
	
};