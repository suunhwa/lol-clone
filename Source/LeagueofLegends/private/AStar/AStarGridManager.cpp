#include "AStar/AStarGridManager.h"
#include "DrawDebugHelpers.h"

AAStarGridManager::AAStarGridManager() { PrimaryActorTick.bCanEverTick = false; }

void AAStarGridManager::BeginPlay()
{
    Super::BeginPlay();
    ScanWorld();
}

void AAStarGridManager::ScanWorld()
{
    // 맵 왼쪽 하단부터 시작해서 격자 생성
    for (float x = -MapSize.X; x <= MapSize.X; x += GridSize)
    {
        for (float y = -MapSize.Y; y <= MapSize.Y; y += GridSize)
        {
            FIntPoint Idx(FMath::RoundToInt(x / GridSize), FMath::RoundToInt(y / GridSize));
            FVector WorldPos(x, y, GetActorLocation().Z);

            FAStarNode Node;
            Node.WorldLocation = WorldPos;
            Node.GridIndex = Idx;

            // 라인트레이스로 벽 감지
            FHitResult Hit;
            FVector End = WorldPos - FVector(0, 0, 500.0f);
            // 벽이 감지되면 못 가는 길로 설정
            Node.bIsWalkable = !GetWorld()->LineTraceSingleByChannel(Hit, WorldPos + FVector(0,0,500.0f), End, ECC_WorldStatic);

            GridMap.Add(Idx, Node);

            // 디버그용 시각화 (빨간색: 벽, 초록색: 길)
            DrawDebugBox(GetWorld(), WorldPos, FVector(GridSize/2.1f), Node.bIsWalkable ? FColor::Green : FColor::Red, true, -1, 0, 2);
        }
    }
}

TArray<FVector> AAStarGridManager::FindPath(FVector StartPos, FVector EndPos)
{
    FIntPoint StartIdx = WorldToGrid(StartPos);
    FIntPoint EndIdx = WorldToGrid(EndPos);

    // 맵에 존재하지 않는 인덱스면 빈 경로 반환
    if (!GridMap.Contains(StartIdx) || !GridMap.Contains(EndIdx)) return TArray<FVector>();
    if (!GridMap[EndIdx].bIsWalkable) return TArray<FVector>(); // 목표가 벽이면 불가
    
    TArray<FIntPoint> OpenList;
    TSet<FIntPoint> ClosedList;
    OpenList.Add(StartIdx);

    // 알고리즘 작동을 위해 노드 데이터 초기화
    for (auto& Elem : GridMap) { Elem.Value.G = 999999.f; }
    GridMap[StartIdx].G = 0;
    GridMap[StartIdx].H = GetDistance(StartIdx, EndIdx);

    while (OpenList.Num() > 0)
    {
        // 1. F값이 가장 낮은 노드 찾기
        FIntPoint CurrentIdx = OpenList[0];
        for (FIntPoint NodeIdx : OpenList)
        {
            if (GridMap[NodeIdx].F() < GridMap[CurrentIdx].F()) CurrentIdx = NodeIdx;
        }

        if (CurrentIdx == EndIdx) break; // 목표 도달

        OpenList.Remove(CurrentIdx);
        ClosedList.Add(CurrentIdx);

        // 2. 주변 8방향 노드 검사
        for (FIntPoint NeighborIdx : GetNeighbors(CurrentIdx))
        {
            if (ClosedList.Contains(NeighborIdx) || !GridMap[NeighborIdx].bIsWalkable) continue;

            float NewG = GridMap[CurrentIdx].G + GetDistance(CurrentIdx, NeighborIdx);
            if (NewG < GridMap[NeighborIdx].G)
            {
                GridMap[NeighborIdx].ParentIndex = CurrentIdx;
                GridMap[NeighborIdx].G = NewG;
                GridMap[NeighborIdx].H = GetDistance(NeighborIdx, EndIdx);
                if (!OpenList.Contains(NeighborIdx)) OpenList.Add(NeighborIdx);
            }
        }
    }

    // 3. 경로 역추적하여 결과 반환
    TArray<FVector> Path;
    FIntPoint TraceIdx = EndIdx;
    while (TraceIdx != StartIdx)
    {
        if (!GridMap.Contains(TraceIdx)) break;
        Path.Insert(GridMap[TraceIdx].WorldLocation, 0);
        TraceIdx = GridMap[TraceIdx].ParentIndex;
    }
    UE_LOG(LogTemp, Warning, TEXT("Path Found! Nodes: %d"), Path.Num());
    return Path;
}

FIntPoint AAStarGridManager::WorldToGrid(FVector WorldPos) { return FIntPoint(FMath::RoundToInt(WorldPos.X / GridSize), FMath::RoundToInt(WorldPos.Y / GridSize)); }
FVector AAStarGridManager::GridToWorld(FIntPoint GridIdx) { return FVector(GridIdx.X * GridSize, GridIdx.Y * GridSize, 0.0f); }
float AAStarGridManager::GetDistance(FIntPoint A, FIntPoint B) { return FVector2D::Distance(FVector2D(A.X, A.Y), FVector2D(B.X, B.Y)); }

TArray<FIntPoint> AAStarGridManager::GetNeighbors(FIntPoint CurrentIdx)
{
    TArray<FIntPoint> Neighbors;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue;
            FIntPoint Next(CurrentIdx.X + x, CurrentIdx.Y + y);
            if (GridMap.Contains(Next)) Neighbors.Add(Next);
        }
    }
    return Neighbors;
}