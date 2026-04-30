#include "Characters/Object/LoLTower.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "LeagueofLegends.h" // PRINTLOG_HJ 등 매크로 포함
#include "Characters/LoLMinion.h"
#include "Interfaces/Targetable.h"
#include "Interfaces/Damageable.h"

ALoLTower::ALoLTower()
{
    // 스탯 초기값 (BeginPlay에서 부모에 의해 덮어씌워짐)
    CurrentTarget = nullptr;
    LastAttackedTarget = nullptr;
    CurrentHeatStack = 0;
}

void ALoLTower::BeginPlay()
{
    // 1. 부모의 BeginPlay 호출 (모든 스탯 변수 할당 완료)
    Super::BeginPlay();

    // 2. 부모가 로드한 AttackSpeed를 기반으로 타이머 인터벌 계산
    // 예: AttackSpeed가 1.2라면 약 0.83초마다 공격
    float FinalInterval = (AttackSpeed > 0.f) ? (1.0f / AttackSpeed) : 1.2f;

    // 3. 공격 루틴 시작
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ALoLTower::CheckAndAttack, FinalInterval, true);

    UE_LOG(LogTemp, Warning, TEXT("[%s] 타워 시스템 가동 - 주기: %.2f초, 공격력: %.1f, 사거리: %.1f"), 
        *GetName(), FinalInterval, AttackDamage, AttackRange);
}

void ALoLTower::CheckAndAttack()
{
    UE_LOG(LogTemp, VeryVerbose, TEXT("[%s] CheckAndAttack 루틴 실행 중..."), *GetName());
    
    if (bIsDestroyed || IsActorBeingDestroyed()) return;

    // 1. 기존 타겟 유효성 검사
    if (IsValid(CurrentTarget))
    {
        float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        IDamageable* Damageable = Cast<IDamageable>(CurrentTarget);
        ITargetable* Targetable = Cast<ITargetable>(CurrentTarget);

        // 타겟이 사거리 밖이거나, 죽었거나, 타겟 불가 상태가 되면 포기
        if (Dist > AttackRange || (Damageable && Damageable->IsDead()) || (Targetable && !Targetable->IsTargetable()))
        {
            CurrentTarget = nullptr;
            CurrentHeatStack = 0; // 타겟 유실 시 가열 스택 초기화
        }
    }

    // 2. 새로운 타겟 탐색 (타겟이 없을 때만 수행)
    if (CurrentTarget == nullptr)
    {
        TArray<AActor*> FoundActors;
        // 타겟팅 시스템용 공통 태그 "Character" 사용
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Character"), FoundActors);

        
        if (FoundActors.Num() == 0)
        {
            // 이 로그가 계속 뜬다면 태그 설정 문제임
            UE_LOG(LogTemp, Warning, TEXT("[%s] 주변에 'Character' 태그를 가진 액터가 하나도 없습니다."), *GetName());
        }
        
        AActor* BestTarget = nullptr;
        int32 BestPriority = -1; // 미니언(2) > 챔피언(1)
        float ClosestDist = AttackRange;

        for (AActor* Actor : FoundActors)
        {
            if (!IsValid(Actor) || Actor == this) continue;

            // 구조물은 공격 대상에서 제외
            if (Actor->IsA(ALoLStructure::StaticClass())) continue;

            ITargetable* Targetable = Cast<ITargetable>(Actor);
            if (Targetable && Targetable->IsTargetable())
            {
                // 적군 팀인지 확인 (MyTeam != TargetTeam)
                if (Targetable->GetTeam() != GetTeam() && Targetable->GetTeam() != ETeam::None)
                {
                    float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                    if (Dist <= AttackRange)
                    {
                        // 미니언 최우선순위 (롤 방식)
                        int32 CurrentPriority = Actor->IsA(ALoLMinion::StaticClass()) ? 2 : 1;

                        if (CurrentPriority > BestPriority || (CurrentPriority == BestPriority && Dist < ClosestDist))
                        {
                            BestPriority = CurrentPriority;
                            ClosestDist = Dist;
                            BestTarget = Actor;
                        }
                    }
                }
            }
        }

        if (BestTarget)
        {
            CurrentTarget = BestTarget;
            // 공격 대상이 바뀌었다면 가열 스택 초기화
            if (CurrentTarget != LastAttackedTarget)
            {
                CurrentHeatStack = 0;
            }
            UE_LOG(LogTemp, Log, TEXT("[%s] 새 타겟 포착: %s"), *GetName(), *CurrentTarget->GetName());
        }
    }

    // 3. 공격 실행
    if (IsValid(CurrentTarget))
    {
        Fire();
    }
}

void ALoLTower::Fire()
{
    if (!IsValid(CurrentTarget)) return;

    // 1. 대미지 계산 로직 (기존과 동일)
    float FinalDamage = AttackDamage;
    if (Max_Heating > 0)
    {
        FinalDamage *= (1.0f + (CurrentHeatStack * Heating_Rate));
    }

    // 2. [시각화] 빨간색 레이저 발사
    DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0, 0, 400), CurrentTarget->GetActorLocation(), FColor::Red, false, 0.2f, 0, 10.0f);

    // 3. [대미지 전달] Execute_ 대신 C++ 직접 캐스팅 사용
    // TObjectPtr이나 AActor*를 인터페이스 포인터로 변환합니다.
    IDamageable* DamageableTarget = Cast<IDamageable>(CurrentTarget);

    if (DamageableTarget)
    {
        // 직접 가상 함수를 호출합니다. (IDamageable에 ReceiveDamage가 구현되어 있어야 함)
        DamageableTarget->ReceiveDamage(FinalDamage, EDamageType::Physical, this);
        
        // 가열 스택 증가 및 마지막 타겟 갱신
        LastAttackedTarget = CurrentTarget;
        CurrentHeatStack = FMath::Min(CurrentHeatStack + 1, Max_Heating);

        UE_LOG(LogTemp, Warning, TEXT("[%s] 발사! -> %s [최종 대미지: %.1f (가열 %d스택)]"), 
            *GetName(), *CurrentTarget->GetName(), FinalDamage, CurrentHeatStack);
    }
    else
    {
        // 타겟이 인터페이스를 상속받지 않았을 경우를 대비한 예외 처리
        UE_LOG(LogTemp, Error, TEXT("[%s] 타겟(%s)이 IDamageable을 구현하지 않았습니다!"), 
            *GetName(), *CurrentTarget->GetName());
    }
}