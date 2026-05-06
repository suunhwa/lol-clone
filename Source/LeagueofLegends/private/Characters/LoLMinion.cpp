#include "Characters/LoLMinion.h"
#include "Manager/MinionDataSubsystem.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"
#include "AStar/AStarGridManager.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"

ALoLMinion::ALoLMinion()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALoLMinion::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    // 1. AStar 매니저 찾기
    GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));

    // 2. 서브시스템을 통한 데이터 초기화 (하드코딩 제거)
    UMinionDataSubsystem* DataSub = GetGameInstance()->GetSubsystem<UMinionDataSubsystem>();
    UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp);

    if (DataSub && MinionStat)
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

            // 컴포넌트 내의 초기화 함수 호출 (성장치 반영)
            MinionStat->InitMinionStats(*BaseRow, *GrowthRow, GameMinutes);
            
            
            // 캐싱
            CachedAttackRange = MinionStat->GetAttackRange();
            CachedAttackSpeed = MinionStat->GetAttackSpeed();
            
            if (MinionStat) 
            {
                UE_LOG(LogTemp, Warning, TEXT("[%s] 데이터 로드 확인 - 사거리: %f"), *GetName(), CachedAttackRange);
            }
            
        }
    }

    // 3. 태그 기본 설정
    if (TagComp)
    {
        TagComp->SetUnitType(EUnitType::Minion);
        TagComp->SetTeam(InitialTeam);
    }
}

void ALoLMinion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SetActorTickEnabled(true);
    
    if (!HasAuthority()) return;

    // 타겟 업데이트 (Aggro 우선순위 1~7번 반영 로직 호출)
    UpdateAggro();

    if (CurrentTarget.IsValid())
    {
        float Dist = FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());

        if (Dist <= CachedAttackRange)
        {
            // 공격 범위 안이면 길찾기 중단 및 공격 (공격 로직은 CombatComp에 위임)
            CurrentPath.Empty();
            // [로그 추가] 공격 범위 진입 확인
            UE_LOG(LogTemp, Log, TEXT("[%s] 공격 사거리 도달!"), *GetName());
            
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
                UE_LOG(LogTemp, Log, TEXT("[%s] 새 경로 요청. 남은 웨이포인트: %d"), *GetName(), CurrentPath.Num());
            }
            MoveAlongPath(DeltaTime);
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
    if (CurrentPath.Num() == 0 || CurrentPathIndex >= CurrentPath.Num()) return;

    FVector TargetPoint = CurrentPath[CurrentPathIndex];
    FVector Direction = (TargetPoint - GetActorLocation()).GetSafeNormal2D();

    // 부모의 CharacterMovementComponent를 이용한 이동
    AddMovementInput(Direction, 1.0f);

    // 방향 보기
    if (!Direction.IsNearlyZero())
    {
        SetActorRotation(Direction.Rotation());
    }

    // 웨이포인트 도달 체크 (그리드 크기 고려하여 약 50cm 내외)
    if (FVector::Dist2D(GetActorLocation(), TargetPoint) < 50.f)
    {
        CurrentPathIndex++;
    }
}

void ALoLMinion::UpdateAggro()
{
    UE_LOG(LogTemp, Log, TEXT("[%s] UpdateAggro 틱 도는 중..."), *GetName());

    AActor* Best = ScanForBestTarget();
    
    if (Best)
    {
        // 타겟이 바뀌었을 때만 출력
        if (CurrentTarget != Best)
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 새 타겟 발견: %s"), *GetName(), *Best->GetName());
            CurrentTarget = Best;
        }
    }
    else
    {
        // 타겟을 못 찾을 때 아주 가끔씩만 로그 출력 (매 프레임 출력하면 렉 걸림)
        static int32 LogCounter = 0;
        if (++LogCounter % 100 == 0) 
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] 주변에 적이 없음 (nullptr)"), *GetName());
        }
    }
}

AActor* ALoLMinion::ScanForBestTarget()
{
    TArray<AActor*> FoundActors;
    
    // ITargetable이 아니라 UTargetable을 넣어야 합니다.
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UTargetable::StaticClass(), FoundActors);
    
    UE_LOG(LogTemp, Log, TEXT("[%s] ITargetable 감지된 액터 수: %d"), *GetName(), FoundActors.Num());
    AActor* ClosestEnemy = nullptr;
    float MinDist = 1500.f;

    for (AActor* Actor : FoundActors)
    {
        if (Actor == this) continue;
        
        // 실제 로직 사용 시에는 다시 ITargetable로 형변환해서 기능을 씁니다.
        ITargetable* TargetInterface = Cast<ITargetable>(Actor);
        
        if (TargetInterface)
        {
            // [로그 추가] 찾은 대상의 팀과 내 팀을 비교
            UE_LOG(LogTemp, Log, TEXT("[%s] 대상: %s, 내 팀: %d, 대상 팀: %d"), 
                *GetName(), *Actor->GetName(), (int32)GetTeam(), (int32)TargetInterface->GetTeam());
            if (TargetInterface->GetTeam() != this->GetTeam())
            {
                float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                if (Dist < MinDist)
                {
                    MinDist = Dist;
                    ClosestEnemy = Actor;
                }
            }
        }
    }
    return ClosestEnemy;
}

int32 ALoLMinion::GetTargetPriority(AActor* PotentialTarget)
{
    // 미니언 우선순위 가이드라인에 따른 정수 반환 (낮을수록 우선)
    return 7; 
}

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