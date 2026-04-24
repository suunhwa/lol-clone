#include "AStar/AStarGridManager.h"
#include "DrawDebugHelpers.h"

AAStarGridManager::AAStarGridManager() { PrimaryActorTick.bCanEverTick = false; }

void AAStarGridManager::BeginPlay() { Super::BeginPlay(); ScanWorld(); }

void AAStarGridManager::ScanWorld()
{
    float BaseZ = 5.0f; // 격자가 바닥에 파묻히지 않게 살짝 올림
    GridMap.Empty(); // 기존 지도 비움

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // 스캔 시 본인은 무시

    // 설정한 MapSize 범위 내를 GridSize 간격으로 바둑판 순회
    for (float x = -MapSize.X; x <= MapSize.X; x += GridSize)
    {
        for (float y = -MapSize.Y; y <= MapSize.Y; y += GridSize)
        {
            // 현재 위치를 격자 번호(인덱스)로 변환 (예: 0,0 / 0,1 ...)
            FIntPoint Idx(FMath::FloorToInt(x / GridSize), FMath::FloorToInt(y / GridSize));
            FVector WorldPos(x, y, BaseZ);

            FAStarNode Node; 
            Node.WorldLocation = WorldPos;
            Node.GridIndex = Idx;

            FHitResult Hit;
            // 허리 높이 스캔, 충돌 검사
            FVector Start = WorldPos + FVector(0, 0, 50.f);
            FVector End = WorldPos + FVector(0, 0, 100.f);
            // 격자 크기의 80% 정도 되는 보이지 않는 박스를 생성
            FCollisionShape Box = FCollisionShape::MakeBox(FVector(GridSize * 0.4f, GridSize * 0.4f, 20.f));
            
            // Sweep 검사 실행 [ 박스를 시작점에서 끝점까지 쭉 밀어봄 ]
            bool bHitObstacle = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, Box, Params);
            
            if (bHitObstacle && Hit.GetActor())
            {
                // 넥서스는 벽이 아님을 명시
                if (Hit.GetActor()->ActorHasTag(TEXT("Structure"))) Node.bIsWalkable = true;
                else Node.bIsWalkable = false; // 그 외 일반 벽이면 못 가는 길
            }
            else { Node.bIsWalkable = !bHitObstacle; }

            GridMap.Add(Idx, Node); // 완성된 그리드 정보를 지도에 저장

            // [디버그] 전체 시각화
            FColor DebugColor = Node.bIsWalkable ? FColor::Green : FColor::Red;
            DrawDebugBox(GetWorld(), WorldPos, FVector(GridSize/2.1f, GridSize/2.1f, 2.0f), DebugColor, true, -1, 0, 1.5f);
        }
    }
}

void AAStarGridManager::ResetNodeIfNewSession(FIntPoint NodeIdx)
{
    // 이번 길찾기에서 처음 쓰인다면
    if (GridMap[NodeIdx].LastSessionID != CurrentSessionID)
    {
        GridMap[NodeIdx].G = 1000000.f; // 아주 큰 값, 즉 아직 안 가본 곳으로 초기화 
        GridMap[NodeIdx].H = 0.f;
        GridMap[NodeIdx].ParentIndex = FIntPoint(-1, -1);
        GridMap[NodeIdx].LastSessionID = CurrentSessionID; // 현재 번호로 갱신
    }
}

TArray<FVector> AAStarGridManager::FindPath(FVector StartPos, FVector EndPos)
{
    CurrentSessionID++; // 길찾기 시작시마다 세션 번호 증가
    FIntPoint StartIdx = WorldToGrid(StartPos);
    FIntPoint EndIdx = WorldToGrid(EndPos);

    // 예외처리 : 시작이나 끝이 지도 밖에 있을 시 중단
    if (!GridMap.Contains(StartIdx) || !GridMap.Contains(EndIdx)) return TArray<FVector>();
    
    // 시작/끝점이 벽에 걸려있어도 길을 찾아야 하므로 잠시 길로 바꿈
    GridMap[StartIdx].bIsWalkable = true;
    GridMap[EndIdx].bIsWalkable = true;

    TArray<FIntPoint> OpenList; // 검사 안한 리스트
    TSet<FIntPoint> ClosedList; // 검사 완료 리스트

    ResetNodeIfNewSession(StartIdx); // 시작 초기화 및 거리 0으로 할당
    GridMap[StartIdx].G = 0;
    GridMap[StartIdx].H = GetDistance(StartIdx, EndIdx);
    OpenList.Add(StartIdx);

    while (OpenList.Num() > 0)
    {
        // [성능 최적화] 후보들 중 F점수(실제거리 G + 예상거리 H)가 가장 낮은 칸을 찾음
        int32 BestNodeIdx = 0;
        for (int32 i = 1; i < OpenList.Num(); ++i)
        {
            if (GridMap[OpenList[i]].F() < GridMap[OpenList[BestNodeIdx]].F())
                BestNodeIdx = i;
        }

        FIntPoint CurrentIdx = OpenList[BestNodeIdx];
        if (CurrentIdx == EndIdx) break;

        OpenList.RemoveAt(BestNodeIdx);
        ClosedList.Add(CurrentIdx);
        
        // 상하좌우대각선 8방향 이웃들을 검사
        for (FIntPoint NeighborIdx : GetNeighbors(CurrentIdx))
        {
            // 이미 검사했으면 패스
            if (ClosedList.Contains(NeighborIdx) || !GridMap[NeighborIdx].bIsWalkable) continue;

            ResetNodeIfNewSession(NeighborIdx); // 노드 초기화
            
            // 대각선 이동 시 가중치 부여 루트계산 (1.4f)
            float MoveCost = (CurrentIdx.X != NeighborIdx.X && CurrentIdx.Y != NeighborIdx.Y) ? 1.4f : 1.0f;
            float NewG = GridMap[CurrentIdx].G + MoveCost;

            // 지금 찾은 길이 기존 알고 있던 길보다 빠르면
            if (NewG < GridMap[NeighborIdx].G)
            {
                GridMap[NeighborIdx].ParentIndex = CurrentIdx;
                GridMap[NeighborIdx].G = NewG;
                GridMap[NeighborIdx].H = GetDistance(NeighborIdx, EndIdx);
                if (!OpenList.Contains(NeighborIdx)) OpenList.Add(NeighborIdx);
            }
        }
    }

    TArray<FVector> Path;
    FIntPoint TraceIdx = EndIdx;
    while (TraceIdx != StartIdx && TraceIdx != FIntPoint(-1, -1))
    {
        Path.Insert(GridMap[TraceIdx].WorldLocation, 0);
        TraceIdx = GridMap[TraceIdx].ParentIndex;
    }
    return Path;
}

FIntPoint AAStarGridManager::WorldToGrid(FVector WorldPos) { return FIntPoint(FMath::FloorToInt(WorldPos.X / GridSize), FMath::FloorToInt(WorldPos.Y / GridSize)); }
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