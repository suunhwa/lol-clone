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

	float G = 0.0f;
	float H = 0.0f;
	FIntPoint ParentIndex = FIntPoint(-1, -1);
	uint32 LastSessionID = 0;

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
	TArray<FVector> FindPath(FVector StartPos, FVector EndPos);

private:
	void ScanWorld(); 
	void ResetNodeIfNewSession(FIntPoint NodeIdx);

	FIntPoint WorldToGrid(FVector WorldPos);
	TArray<FIntPoint> GetNeighbors(FIntPoint CurrentIdx);
	float GetDistance(FIntPoint A, FIntPoint B);

	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	float GridSize = 40.0f; // 촘촘하게 설정 (40cm 단위)

	UPROPERTY(EditAnywhere, Category = "Grid Settings")
	FVector2D MapSize = FVector2D(5000.0f, 5000.0f);

	TMap<FIntPoint, FAStarNode> GridMap;
	uint32 CurrentSessionID = 0;
};