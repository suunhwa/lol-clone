#include "Characters/Object/LoLTower.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ALoLTower::ALoLTower()
{
    // 생성자에서는 기본적인 초기화만 수행
    AttackDamage = 0.f;
    AttackRange = 0.f;
    AttackInterval = 1.2f;
}

void ALoLTower::BeginPlay()
{
    // 1. 부모의 BeginPlay 호출 (여기서 데이터 테이블 로드가 일어남)
    Super::BeginPlay();

    // 2. 부모가 로드한 StatData(FObjectBaseRow)를 사용하여 타워 스탯 설정
    // StatData는 부모 클래스인 ALoLStructure에 선언되어 있음
    if (StatData.Object_ID != 0)
    {
        AttackDamage = StatData.Base_AD;
        AttackRange = StatData.Atk_Range;
        
        // 롤의 Atk_Speed는 초당 공격 횟수이므로, 타이머용 인터벌(초)로 변환
        AttackInterval = (StatData.Atk_Speed > 0.f) ? (1.0f / StatData.Atk_Speed) : 1.2f;
    }

    // 3. 설정된 인터벌로 공격 타이머 시작
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ALoLTower::CheckAndAttack, AttackInterval, true);
}

void ALoLTower::CheckAndAttack()
{
    // 0. 타워 자체가 파괴 중인지 확인 (IsActorBeingDestroyed 사용)
    if (IsActorBeingDestroyed() || bIsDestroyed) return;

    // 1. 기존 타겟 유효성 검사
    // IsValid()는 null 체크와 PendingKill(Garbage) 체크를 동시에 수행합니다.
    if (IsValid(CurrentTarget))
    {
        float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        IDamageable* Damageable = Cast<IDamageable>(CurrentTarget);
        
        // 타겟이 범위 밖이거나, 죽었거나, 혹은 제거 중(BeingDestroyed)인지 확인
        if (Dist > AttackRange || (Damageable && Damageable->IsDead()) || CurrentTarget->IsActorBeingDestroyed())
        {
            UE_LOG(LogTemp, Display, TEXT("[%s] 타겟 유실: %s (거리: %.1f)"), *GetName(), *CurrentTarget->GetName(), Dist);
            CurrentTarget = nullptr;
            CurrentHeatStack = 0; 
        }
    }
    else
    {
        // 타겟이 이미 GC에 의해 소멸되었거나 nullptr인 경우
        CurrentTarget = nullptr;
        CurrentHeatStack = 0;
    }

    // 2. 새로운 타겟 찾기
    if (CurrentTarget == nullptr)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Character"), FoundActors);

        float ClosestDist = AttackRange;
        FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;

        for (AActor* Actor : FoundActors)
        {
            // [최신 방식] IsValid()로 유효성 및 소멸 여부 동시 체크
            if (!IsValid(Actor) || Actor == this) continue;

            ITargetable* Targetable = Cast<ITargetable>(Actor);
            if (Targetable && Targetable->GetTeam() != GetTeam() && Targetable->IsTargetable())
            {
                float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                if (Dist <= ClosestDist)
                {
                    ClosestDist = Dist;
                    CurrentTarget = Actor;
                }
            }
        }
        if (IsValid(CurrentTarget))
        {
            UE_LOG(LogTemp, Warning, TEXT("[%s] 신규 타겟 포착! -> %s"), *GetName(), *CurrentTarget->GetName());
        }
        
    }

    // 3. 타겟이 확정되었다면 발사
    if (IsValid(CurrentTarget))
    {
        Fire();
    }
}

void ALoLTower::Fire()
{
    if (!CurrentTarget) return;

    // [가열 시스템 예시] 스택당 데미지 증가 (MechData 활용)
    float FinalDamage = AttackDamage;
    if (MechData.Max_Heating > 0)
    {
        FinalDamage *= (1.0f + (CurrentHeatStack * MechData.Heating_Rate));
        CurrentHeatStack = FMath::Min(CurrentHeatStack + 1, MechData.Max_Heating);
    }
    // 시각화
    FVector StartLocation = GetActorLocation() + FVector(0, 0, 200);
    FVector EndLocation = CurrentTarget->GetActorLocation();
    
    // [시각화]
    DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0,0,200), CurrentTarget->GetActorLocation(), FColor::Red, false, 0.2f, 0, 5.0f);

    // [로그] 공격 상세 정보
    UE_LOG(LogTemp, Warning, TEXT("[%s] Fire! -> %s [데미지: %.1f (가열 %d스택)]"), 
        *GetName(), *CurrentTarget->GetName(), FinalDamage, CurrentHeatStack);
    
    // [데미지 입히기]
    IDamageable* DamageableTarget = Cast<IDamageable>(CurrentTarget);
    if (DamageableTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] 공격 -> %s (데미지: %.1f, 스택: %d)"), 
            *GetName(), *CurrentTarget->GetName(), FinalDamage, CurrentHeatStack);
            
        DamageableTarget->ReceiveDamage(FinalDamage, EDamageType::Physical, this);
    }
}