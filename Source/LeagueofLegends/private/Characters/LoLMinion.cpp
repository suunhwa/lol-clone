#include "Characters/LoLMinion.h"

#include "LeagueofLegends.h"
#include "Manager/MinionDataSubsystem.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"
#include "AStar/AStarGridManager.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"

ALoLMinion::ALoLMinion()
{
    PrimaryActorTick.bCanEverTick = true;
    
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ALoLMinion::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    // AStar 매니저 찾기
    GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));

    // 데이터 초기화
    if (UMinionDataSubsystem* DataSub = GetGameInstance()->GetSubsystem<UMinionDataSubsystem>())
    {
        if (UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp))
        {
            auto* BaseRow = DataSub->GetBaseRowByID(MinionID);
            auto* GrowthRow = DataSub->GetGrowthRowByID(MinionID);

            if (BaseRow && GrowthRow)
            {
                int32 GameMinutes = 0;
                if (ARiftGameState* GS = GetWorld()->GetGameState<ARiftGameState>())
                {
                    GameMinutes = FMath::FloorToInt(GS->GetServerWorldTimeSeconds() / 60.f);
                }

                MinionStat->InitMinionStats(*BaseRow, *GrowthRow, GameMinutes);
                
                // 데이터 기반 수치 로드 (데이터 테이블 최선 순위)
                CachedAttackRange = MinionStat->GetAttackRange();
                CachedAttackSpeed = MinionStat->GetAttackSpeed();
                
                // 만약 데이터 테이블에 어그로 범위 컬럼을 추가했다면 여기서 로드 가능
                // AcquisitionRange = BaseRow->AcqRange; 
            }
        }
    }

    if (TagComp)
    {
        TagComp->SetUnitType(EUnitType::Minion);
        if (TagComp->GetTeam() == ETeam::None) TagComp->SetTeam(InitialTeam);
    }
}

void ALoLMinion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!HasAuthority()) return;
    
    // 1. 타겟팅 업데이트 (주기적 실행)
    UpdateAggro(DeltaTime);

    if (CurrentTarget.IsValid())
    {
        float Dist = FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());

        // 어그로 해제 거리 체크 (너무 멀어지면 타겟 상실)
        if (Dist > LoseTargetRange)
        {
            CurrentTarget = nullptr;
            CurrentPath.Empty();
            return;
        }
        
        if (Dist <= CachedAttackRange)
        {
            // 공격 범위 안이면 길찾기 중단 및 공격 (공격 로직은 CombatComp에 위임)
            CurrentPath.Empty();
            // TODO: 요기서 이제 전투컴포붙여서 공격로직
            // [로그 추가] 공격 범위 진입 확인
            PRINTLOG_HJ(TEXT("[%s] 공격 사거리 도달!"), *GetName());
        }
        else
        {
            // 공격 범위 밖이면 A* 추적
            PathUpdateTimer += DeltaTime;
            if (PathUpdateTimer >= PathUpdateInterval)
            {
                RequestNewPath(CurrentTarget->GetActorLocation());
                PathUpdateTimer = 0.f;
                
                // [로그 추가] 경로 생성 확인
                //PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 새 경로 요청. 남은 웨이포인트: %d"), *GetName(), CurrentPath.Num());
            }
            MoveAlongPath(DeltaTime);
        }
    }
    else if (LanePath.Num() > 0)
    {
        // --- [공격로 전진 로직] 타겟이 없을 때 실행 ---
        if (LanePath.IsValidIndex(CurrentLaneIndex))
        {
            FVector TargetPoint = LanePath[CurrentLaneIndex];
            float DistToWP = FVector::Dist2D(GetActorLocation(), TargetPoint);

            // 1. 체크포인트 도달 판정 (1.5미터 이내)
            if (DistToWP < 150.f) 
            {
                CurrentLaneIndex++; // 다음 점으로!
            }
            else
            {
                // 2. 다음 점을 향해 이동
                PathUpdateTimer += DeltaTime;
                if (PathUpdateTimer >= PathUpdateInterval)
                {
                    RequestNewPath(TargetPoint);
                    PathUpdateTimer = 0.f;
                }
                MoveAlongPath(DeltaTime);
            }
        }
    }
}

void ALoLMinion::RequestNewPath(FVector Destination)
{
    if (GridManager)
    {
        // 제공해주신 AStarGridManager의 FindPath 사용
        CurrentPath = GridManager->FindPath(GetActorLocation(), Destination);
        CurrentPathIndex = 0;
    }
}

void ALoLMinion::MoveAlongPath(float DeltaTime)
{
    if (CurrentPath.Num() == 0 || CurrentPathIndex >= CurrentPath.Num())
    {
        return;
    }
    
    FVector TargetPoint = CurrentPath[CurrentPathIndex];
    FVector Direction = (TargetPoint - GetActorLocation()).GetSafeNormal2D();
    // 부모의 CharacterMovementComponent를 이용한 이동
    AddMovementInput(Direction, 1.0f);

    // 방향 보기
    if (!Direction.IsNearlyZero())
    {
        SetActorRotation(Direction.Rotation());
    }

    // 웨이포인트 도달 체크 [그리드 크기 고려하여 약 50cm 내외]
    if (FVector::Dist2D(GetActorLocation(), TargetPoint) < 50.f)
    {
        CurrentPathIndex++;
    }
}

void ALoLMinion::UpdateAggro(float DeltaTime)
{
    AggroUpdateTimer += DeltaTime;
    if (AggroUpdateTimer >= AggroUpdateInterval)
    {
        AActor* Best = ScanForBestTarget();
        if (Best && CurrentTarget != Best)
        {
            CurrentTarget = Best;
            PRINTLOG_HJ(TEXT("[%s] 새 타겟 발견: %s"), *GetName(), *Best->GetName());
        }
        AggroUpdateTimer = 0.f;
    }
}

AActor* ALoLMinion::ScanForBestTarget()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UTargetable::StaticClass(), FoundActors);
    
    // PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] ITargetable 감지된 액터 수: %d"), *GetName(), FoundActors.Num());
    AActor* BestTarget = nullptr;
    int32 BestPriority = 100;
    float ClosestDist = MAX_FLT;

    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == this) continue;
        
        // 팀 확인 (TagComp 활용)
        if (ITargetable::Execute_GetTeam(Actor) == TagComp->GetTeam()) continue;

        // 생존 확인 (태그 활용)
        if (UTagComponent* TargetTag = Actor->FindComponentByClass<UTagComponent>())
        {
            if (TargetTag->HasTag(UnitTags::Dead)) continue;
        }

        float Dist = FVector::Dist2D(GetActorLocation(), Actor->GetActorLocation());
        if (Dist > AcquisitionRange) continue;

        int32 Priority = GetTargetPriority(Actor);

        if (Priority < BestPriority)
        {
            BestPriority = Priority;
            ClosestDist = Dist;
            BestTarget = Actor;
        }
        else if (Priority == BestPriority)
        {
            if (Dist < ClosestDist)
            {
                ClosestDist = Dist;
                BestTarget = Actor;
            }
        }
    }
    return BestTarget;
}

int32 ALoLMinion::GetTargetPriority(AActor* PotentialTarget)
{
    if (!PotentialTarget) return 100;

    EUnitType TargetType = ITargetable::Execute_GetUnitType(PotentialTarget);
    AActor* TargetsTarget = ITargetable::Execute_GetCurrentCombatTarget(PotentialTarget);

    EUnitType AllyType = EUnitType::Minion; 
    ETeam AllyTeam = ETeam::None;

    if (TargetsTarget && TargetsTarget->Implements<UTargetable>())
    {
        AllyTeam = ITargetable::Execute_GetTeam(TargetsTarget);
        AllyType = ITargetable::Execute_GetUnitType(TargetsTarget);
    }

    bool bAttackingMyTeam = (AllyTeam == TagComp->GetTeam());

    // --- LoL 7단계 우선순위 로직 ---
    if (TargetType == EUnitType::Champion && bAttackingMyTeam && AllyType == EUnitType::Champion) return 1;
    if (TargetType == EUnitType::Minion && bAttackingMyTeam && AllyType == EUnitType::Champion)   return 2;
    if (TargetType == EUnitType::Minion && bAttackingMyTeam && AllyType == EUnitType::Minion)     return 3;
    if (TargetType == EUnitType::Tower && bAttackingMyTeam && AllyType == EUnitType::Minion)      return 4;
    if (TargetType == EUnitType::Champion && bAttackingMyTeam && AllyType == EUnitType::Minion)   return 5;
    if (TargetType == EUnitType::Minion)   return 6;
    if (TargetType == EUnitType::Champion) return 7;

    return 8; 
}

// --- Interface 구현부 ---
ETeam ALoLMinion::GetTeam_Implementation() const { return TagComp ? TagComp->GetTeam() : ETeam::None; }
EUnitType ALoLMinion::GetUnitType_Implementation() const { return EUnitType::Minion; }
AActor* ALoLMinion::GetCurrentCombatTarget_Implementation() const { return CurrentTarget.Get(); }
/*// 포탑이 호출하는 ReceiveDamage를 미니언의 HP 로직으로 연결
void ALoLMinion::ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
	// 부모 클래스의 HasAuthority() 체크를 우회하여 즉시 대미지 적용
	// 미니언 클래스에 이미 만들어둔 TakeDamageSimple 함수를 호출합니다.
	TakeDamageSimple(Amount);
}

// 태그를 기반으로 정확한 팀 정보를 반환
ETeam ALoLMinion::GetTeam_Implementation() const
{
	if (Tags.Contains(TEXT("RedTeam"))) return ETeam::Red;
	if (Tags.Contains(TEXT("BlueTeam"))) return ETeam::Blue;
	return ETeam::None;
}*/