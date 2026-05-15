#include "Characters/LoLMinion.h"

#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "FOW/FOWManager.h"
#include "Manager/MinionDataSubsystem.h"
#include "Components/MinionStatComponent.h"
#include "Components/TagComponent.h"
#include "AStar/AStarGridManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RiftGameMode.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"

ALoLMinion::ALoLMinion()
{
    PrimaryActorTick.bCanEverTick = true;
    
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    
    // --- 튕김 방지 설정 ---
    // 1. 캐릭터 무브먼트 컴포넌트 가져오기
    if (auto* MoveComp = GetCharacterMovement())
    {
        // 캡슐끼리 겹쳤을 때 강하게 튕겨내지 않고 부드럽게 밀어내게 함
        MoveComp->MaxDepenetrationWithPawn = 5.f; 
        
        // 미니언끼리 길막을 덜 하도록 설정
        MoveComp->bUseRVOAvoidance = true; // RVO 회피 활성화 (추천)
        MoveComp->AvoidanceConsiderationRadius = 80.f;
        MoveComp->AvoidanceWeight = 0.5f;
        
        //  상호작용 끄기
        MoveComp->bEnablePhysicsInteraction = false;
    }

    // 2. 캡슐 물리 설정 (Physics에 의한 튕김 제거)
    if (auto* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCanEverAffectNavigation(false);
        // 물리 시뮬레이션은 끄고, 단순 Overlap/Block만 사용
        Capsule->SetSimulatePhysics(false); 
        
        Capsule->SetCollisionProfileName(TEXT("Pawn"));
    }
    
}

void ALoLMinion::BeginPlay()
{
    Super::BeginPlay();
    
    // [진단 로그]
    PRINTLOG_HJ(TEXT("[%s] BeginPlay 시작! 현재 MinionID: %d"), *GetName(), MinionID);
    
    if (!HasAuthority()) { return; }

    // AStar 매니저 찾기
    GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));

    // 데이터 초기화
    if (UMinionDataSubsystem* DataSub = GetGameInstance()->GetSubsystem<UMinionDataSubsystem>())
    {
        if (UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp))
        {
            auto* BaseRow = DataSub->GetBaseRowByID(MinionID);
            if (!BaseRow)
            {
                PRINTLOG_HJ(TEXT("!! BaseRow 로드 실패 (ID: %d) !!"), MinionID);
            }
            
            auto* GrowthRow = DataSub->GetGrowthRowByID(MinionID);
            if (!GrowthRow)
            {
                PRINTLOG_HJ(TEXT("!! GrowthRow 로드 실패 (ID: %d) !!"), MinionID);
            }
            
            if (BaseRow && GrowthRow)
            {
                int32 GameMinutes = 0;
                if (ARiftGameState* GS = GetWorld()->GetGameState<ARiftGameState>())
                {
                    GameMinutes = FMath::FloorToInt(GS->GetServerWorldTimeSeconds() / 60.f);
                }
                MinionStat->SetLevel(1);
                MinionStat->InitMinionStats(*BaseRow, *GrowthRow, GameMinutes);
                
                // 데이터 기반 수치 로드 (데이터 테이블 최선 순위)
                CachedAttackRange = MinionStat->GetAttackRange();
                CachedAttackSpeed = MinionStat->GetAttackSpeed();
                
                // 3. [개선] MinionID로 타입 이름 결정 (3001:전사, 3002:법사, 3003:공성, 3004:슈퍼)
                FString MinionTypeName;
                switch (MinionID)
                {
                case 3001: MinionTypeName = TEXT("전사"); break;
                case 3002: MinionTypeName = TEXT("법사"); break;
                case 3003: MinionTypeName = TEXT("공성"); break;
                case 3004: MinionTypeName = TEXT("슈퍼"); break;
                default:   MinionTypeName = TEXT("알수없음"); break;
                }

                // 4. 이제 로그 출력 (정확한 시점!)
                PRINTLOG_HJ(TEXT("[소환 완료] %s (%d) - HP: %.0f/%.0f, AD: %.1f"), 
                    *MinionTypeName, MinionID, MinionStat->GetCurrentHP(), MinionStat->GetMaxHP(), MinionStat->GetAD());
            }
        }
    }

    if (TagComp)
    {
        TagComp->SetUnitType(EUnitType::Minion);
        /*if (TagComp->GetTeam() == ETeam::None)
        {
            TagComp->SetTeam(InitialTeam);
        }*/
        // [수정] 팀 설정 확인 로그 추가
        PRINTLOG_HJ(TEXT("[%s] 소환됨 - 팀: %d (0:None, 1:Blue, 2:Red)"), 
            *GetName(), (int32)TagComp->GetTeam());
        
    }
    
    if (StatComp)
    {
        StatComp->OnHPChanged.AddLambda([this](float CurrentHP, float MaxHP) {
            if (CurrentHP > 0)
            {
                PRINTLOG_HJ(TEXT("[%s] 체력 변동 - 현재 HP: %.1f"), *GetName(), CurrentHP);
            }
        });
    }
    
}

void ALoLMinion::OnDeath(AActor* DamageInstigator)
{
    if (HasAuthority())
    {
        // CS 추적: 챔피언이 죽인 경우
        if (ALoLChampion* KillerChamp = Cast<ALoLChampion>(DamageInstigator))
        {
            if (ARiftPlayerState* PS = KillerChamp->GetPlayerState<ARiftPlayerState>())
            {
                PS->AddCS(1);
            }
        }

        // XP 분배
        FName RowName = GetExpRowName();
        if (!RowName.IsNone())
        {
            if (ARiftGameMode* GM = GetWorld()->GetAuthGameMode<ARiftGameMode>())
            {
                ETeam MyTeam = ITargetable::Execute_GetTeam(this);
                ETeam KillerTeam = (MyTeam == ETeam::Blue) ? ETeam::Red : ETeam::Blue;
                GM->OnUnitKilled(RowName, GetActorLocation(), KillerTeam);
            }
        }
    }

    CurrentTarget = nullptr;
    CurrentPath.Empty();

    // FOW 시야 제공자 해제
    if (ARiftGameState* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        if (AFOWManager* FOW = GS->GetFOWManager())
        {
            FOW->UnregisterSightProvider(this);
        }
    }

    Super::OnDeath(DamageInstigator);
    SetLifeSpan(0.3f);
}

void ALoLMinion::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!HasAuthority()) { return; }
    if (StatComp && StatComp->IsDead()) { return; } // 사망 후 AI 중단

    // 1. 타겟팅 업데이트 (주기적 실행)
    UpdateAggro(DeltaTime);

    if (CurrentTarget.IsValid())
    {
        if (IDamageable::Execute_IsDead(CurrentTarget.Get()))
        {
            CurrentTarget = nullptr;
            CurrentPath.Empty();
            return;
            
        }
        float Dist = FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());

        // 어그로 해제 거리 체크 (너무 멀어지면 타겟 상실)
        if (Dist > LoseTargetRange)
        {
            CurrentTarget = nullptr;
            CurrentPath.Empty();
            return;
        }
        
        float RealRange = (StatComp) ? StatComp->GetAttackRange() : 0.f;
        
        
        if (Dist <= (RealRange + 100.f))
        {
            
            // 공격 범위 안이면 길찾기 중단 및 공격 (공격 로직은 CombatComp에 위임)
            CurrentPath.Empty();
            // 타겟 방향으로 회전
            FVector Dir = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), Dir.Rotation(), DeltaTime, 10.f));

            // 공격 쿨타임 체크 (StatComp의 GetAttackSpeed 이용)
            float CurrentTime = GetWorld()->GetTimeSeconds();
            if (CurrentTime - LastAttackTime >= GetAttackCooldown())
            {
                // 여기서 ExecuteAttack 호출 전 한 번 더 검증
                if (IsValid(CurrentTarget.Get()))
                {
                    ExecuteAttack();
                    LastAttackTime = CurrentTime;
                }
            }
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
                // PRINTLOG_HJ(LogTemp, Log, TEXT("[%s] 새 경로 요청. 남은 웨이포인트: %d"), *GetName(), CurrentPath.Num());
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

float ALoLMinion::GetAttackCooldown() const
{
    // StatComp(부모타입)의 GetAttackSpeed()를 호출하여 계산
    float AS = StatComp ? StatComp->GetAttackSpeed() : 1.0f;
    return 1.0f / FMath::Max(AS, 0.1f);
}

void ALoLMinion::ExecuteAttack()
{   
    // 1. 모든 필수 요소가 유효한지 한 번에 검사 (가장 안전함)
    if (!IsValid(this) || !CombatComp || !StatComp || !CurrentTarget.IsValid())
    {
        return; 
    }

    // 2. 타겟이 월드에서 제거 중이거나 죽었는지 검사
    AActor* TargetActor = CurrentTarget.Get();
    if (!IsValid(TargetActor) || IDamageable::Execute_IsDead(TargetActor))
    {
        CurrentTarget = nullptr;
        return;
    }

    float Damage = StatComp->GetAD();

    if (StatComp->GetAttackRange() > 200.f)
    {
        // TODO: 원거리 투사체 소환
        PRINTLOG_HJ(TEXT("[%s] 원거리 투사체 발사! 데미지: %.1f"), *GetName(), Damage);
    }
    else
    {
        // [수정] 대상이 Damageable 인터페이스를 구현했는지 안전하게 확인 후 호출
        if (CurrentTarget->Implements<UDamageable>())
        {
            IDamageable::Execute_ReceiveDamage(CurrentTarget.Get(), Damage, EDamageType::Physical, this);
            //PRINTLOG_HJ(TEXT("[%s] 근접 공격! %s에게 %.1f 데미지"), *GetName(), *CurrentTarget->GetName(), Damage);
        }
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    // CombatComponent를 통한 실제 공격 로직 수행
    CombatComp->PerformBasicAttack(CurrentTarget.Get());
}

FName ALoLMinion::GetExpRowName() const
{
    if (UMinionDataSubsystem* DataSub = GetGameInstance()->GetSubsystem<UMinionDataSubsystem>())
    {
        if (const FMinionBaseRow* Row = DataSub->GetBaseRowByID(MinionID))
        {
            // return Row->ExpRowName;
            if (!Row->ExpRowName.IsNone())
            {
                return Row->ExpRowName;
            }
        }
    }
    // return NAME_None;
    
    // DataTable에 ExpRowName 미설정 시 MinionID 기반 폴백
    switch (MinionID)
    {
    case 3001: return FName("Minion_Melee");
    case 3002: return FName("Minion_Ranged");
    case 3003: return FName("Minion_Siege");
    case 3004: return FName("Minion_Super");
    default:   return NAME_None;
    }
}

void ALoLMinion::ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
    // 죽은 상태면 무시
    if (!StatComp || StatComp->IsDead()) return;

    // 로그 찍어서 대미지가 오는지 확인
    PRINTLOG_HJ(TEXT("[%s] 포탑/챔피언으로부터 %.1f 대미지 수신!"), *GetName(), Amount);

    // 실제 대미지 적용 (StatComponent의 HP 감소 호출)
    // 기존 TakeDamage와 로직을 통일하기 위해 내부적으로 처리
    StatComp->ApplyHealthChange(-Amount);

    // 사망 체크
    if (StatComp->IsDead())
    {
        Die(DamageInstigator);
    }
}

float ALoLMinion::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || !StatComp || StatComp->IsDead()) return 0.f;

    float ActualDamage = DamageAmount;
    
    PRINTLOG_HJ(TEXT("[%s] 피격! 받은데미지: %.1f, 현재HP: %.1f"), 
        *GetName(), ActualDamage, StatComp->GetCurrentHP());
        
    // [미니언 전용 로직 적용] 미니언 스탯 컴포넌트를 참조하여 특수 방어 효과 적용
    if (UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp))
    {
        // 예: 포탑으로부터 받은 데미지라면 감소 로직 적용
        // if (DamageCauser->IsA(ATower::StaticClass())) 
        //     ActualDamage *= (1.0f - MinionStat->GetTowerDamageReduction());
    }

    // 부모 StatComponent의 체력 변경 함수 호출
    StatComp->ApplyHealthChange(-ActualDamage);

    // [데미지 로그 추가]
    FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("Unknown");
    PRINTLOG_HJ(TEXT("[%s] 피격! 데미지: %.1f | 남은 체력: %.1f / %.0f (가해자: %s)"), 
        *GetName(), ActualDamage, StatComp->GetCurrentHP(), StatComp->GetMaxHP(), *CauserName);
    
    
    if (StatComp->IsDead())
    {
        Die(DamageCauser);
    }

    return ActualDamage;
}

void ALoLMinion::Die(AActor* Killer)
{
    // 1. 태그 변경 (사망 상태 표시)
    // if (TagComp) TagComp->AddTag(UnitTags::Dead);
    if (TagComp)
    {
        TagComp->AddTag(UnitTags::Dead);
        TagComp->AddTag(UnitTags::Untargetable);
    }
    
    // 죽는 순간 즉시 충돌을 꺼버려서 길막/튕김 원천 봉쇄
    if (auto* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
    /*// 2. 골드 보상 로직 (미니언스탯컴포넌트 참조)
    if (UMinionStatComponent* MinionStat = Cast<UMinionStatComponent>(StatComp))
    {
        // Killer에게 골드 지급 로직 수행 (GetBaseGoldReward() 사용)
        // GiveGoldToActor(Killer, MinionStat->GetBaseGoldReward());
    }

    // 3. 소멸 (애니메이션이 있다면 딜레이 후 소멸)
    PRINTLOG_HJ(TEXT("[%s] 사망했습니다."), *GetName());
    Destroy();*/
    
    // OnDeath 체인 실행 (CS, XP, FOW 해제 등 처리)
    CombatComp->OnDeath.Broadcast(Killer);
}

// --- Interface 구현부 ---
ETeam ALoLMinion::GetTeam_Implementation() const { return TagComp ? TagComp->GetTeam() : ETeam::None; }
EUnitType ALoLMinion::GetUnitType_Implementation() const { return EUnitType::Minion; }
AActor* ALoLMinion::GetCurrentCombatTarget_Implementation() const { return CurrentTarget.Get(); }