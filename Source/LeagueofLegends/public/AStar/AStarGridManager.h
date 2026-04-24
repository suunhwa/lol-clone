#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarGridManager.generated.h"

USTRUCT(BlueprintType)
struct FAStarNode
{
	GENERATED_BODY()

	FVector WorldLocation = FVector::ZeroVector;
	FIntPoint GridIndex = FIntPoint(-1, -1);
	bool bIsWalkable = true;

	float G = 0.0f; // 시작점부터의 비용
	float H = 0.0f; // 목적지까지의 예상 비용
	FIntPoint ParentIndex = FIntPoint(-1, -1);

	float F() const { return G + H; }
};

UCLASS()
class LEAGUEOFLEGENDS_API AAStarGridManager : public AActor
{
	GENERATED_BODY()

public:
	AAStarGridManager();

protected:
	virtual void BeginPlay() override;

public:
	// 미니언이 호출할 핵심 함수
	TArray<FVector> FindPath(FVector StartPos, FVector EndPos);

private:
	void ScanWorld(); // 맵 전체 스캔, 벽 감지
    
	FIntPoint WorldToGrid(FVector WorldPos);
	FVector GridToWorld(FIntPoint GridIdx);
	TArray<FIntPoint> GetNeighbors(FIntPoint CurrentIdx);
	float GetDistance(FIntPoint A, FIntPoint B);

	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	float GridSize = 100.0f; // 1미터 단위 격자

	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	FVector2D MapSize = FVector2D(5000.0f, 5000.0f); // 맵 가로세로 범위

	TMap<FIntPoint, FAStarNode> GridMap;
};