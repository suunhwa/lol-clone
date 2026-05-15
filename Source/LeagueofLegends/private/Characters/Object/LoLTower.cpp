#include "Characters/Object/LoLTower.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ALoLTower::ALoLTower()
{
    ObjectID = 11001; 
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(GetRootComponent());
    FirePoint->SetRelativeLocation(FVector(0.f, 0.f, 400.f)); 
}

void ALoLTower::BeginPlay()
{
    Super::BeginPlay();
    
    // 공격 속도 로드 및 타이머 시작
    float AttackSpeed = ObjectStatComp->GetAttackSpeed();
    float Interval = (AttackSpeed > 0.f) ? (1.f / AttackSpeed) : 1.2f;
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ALoLTower::CheckAndAttack, Interval, true);
}

void ALoLTower::CheckAndAttack()
{
    if (ObjectStatComp->IsDead())
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        return;
    }

    bool bNeedNewTarget = true;

    // 1. 기존 타겟 검증 (인터페이스 포인터 캐스팅 절대 금지)
    if (IsValid(CurrentTarget))
    {
        if (CurrentTarget->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
        {
            float Distance = GetDistanceTo(CurrentTarget);
            bool bIsDead = IDamageable::Execute_IsDead(CurrentTarget);

            if (!bIsDead && Distance <= ObjectStatComp->GetAttackRange())
            {
                bNeedNewTarget = false;
            }
        }
    }

    // 2. 새 타겟 탐색
    if (bNeedNewTarget)
    {
        CurrentTarget = nullptr;
        ObjectStatComp->ResetHeatingStack();
        SearchTarget();
    }

    // 3. 타겟이 있으면 발사
    if (IsValid(CurrentTarget))
    {
        Fire();
    }
}

void ALoLTower::SearchTarget()
{
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
    
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    TArray<AActor*> OutActors;
    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), ObjectStatComp->GetAttackRange(), 
        ObjectTypes, nullptr, IgnoreActors, OutActors);

    float ClosestDist = MAX_FLT;
    ETeam MyTeam = ITargetable::Execute_GetTeam(this); // 나 자신의 팀도 Execute로 가져옴

    for (AActor* OverlapActor : OutActors)
    {
        if (!IsValid(OverlapActor)) continue;

        // 인터페이스 구현 여부 확인
        if (OverlapActor->GetClass()->ImplementsInterface(UTargetable::StaticClass()) &&
            OverlapActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
        {
            ETeam TargetTeam = ITargetable::Execute_GetTeam(OverlapActor);
            bool bIsDead = IDamageable::Execute_IsDead(OverlapActor);

            if (TargetTeam != MyTeam && !bIsDead)
            {
                float Dist = GetDistanceTo(OverlapActor);
                if (Dist < ClosestDist)
                {
                    ClosestDist = Dist;
                    CurrentTarget = OverlapActor;
                }
            }
        }
    }
}

void ALoLTower::Fire()
{
    if (!ProjectileClass || !IsValid(CurrentTarget)) return;

    FVector SpawnLocation = FirePoint->GetComponentLocation();
    FRotator SpawnRotation = (CurrentTarget->GetActorLocation() - SpawnLocation).Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;

    if (ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params))
    {
        float FinalDamage = ObjectStatComp->GetAttackDamage(CurrentTarget);
        Projectile->Launch(CurrentTarget, 1500.f, FinalDamage, ITargetable::Execute_GetTeam(this));

        // 챔피언 가열 로직 - 인터페이스 포인터 사용 금지
        if (CurrentTarget->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
        {
            if (ITargetable::Execute_GetUnitType(CurrentTarget) == EUnitType::Champion)
            {
                ObjectStatComp->AddHeatingStack();
                PRINTLOG_HJ(TEXT("[포탑] 챔피언 공격! 가열 스택: %d"), ObjectStatComp->GetCurrentHeatStack());
            }
            else
            {
                ObjectStatComp->ResetHeatingStack();
            }
        }
    }
}


void ALoLTower::OnDestroyed()
{
    // 1. 부모의 공통 정리(상태값 변경 등) 호출
    Super::OnDestroyed();

    // 2. 타워 공격 타이머 끄기
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);

    /*// 3. 타워는 다시 살아나지 않으므로 파괴
    if (HasAuthority())
    {
        Destroy();
    }*/
}