#include "Characters/Object/LoLTower.h"
#include "Characters/Minion/Ranged_Projectile.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ALoLTower::ALoLTower()
{
    ObjectID = 11001; // 기본 ID (에디터 인스턴스에서 11002, 11003 등으로 수정 가능)
    
    FirePoint = CreateDefaultSubobject<USceneComponent>(TEXT("FirePoint"));
    FirePoint->SetupAttachment(RootComponent);
    // 포탑 머리 끝 위치로 기본값 설정 (에디터에서 조정 권장)
    FirePoint->SetRelativeLocation(FVector(0.f, 0.f, 400.f)); 
}

void ALoLTower::BeginPlay()
{
    Super::BeginPlay();
    
    // 공격 속도에 따른 타이머 설정
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

    // 1. 기존 타겟 검증 (죽었거나, 사거리 밖이거나, 유효하지 않으면 제거)
    bool bNeedNewTarget = true;
    if (IsValid(CurrentTarget))
    {
        IDamageable* DamageableTarget = Cast<IDamageable>(CurrentTarget);
        float Distance = GetDistanceTo(CurrentTarget);

        if (DamageableTarget && !DamageableTarget->Execute_IsDead(CurrentTarget) && Distance <= ObjectStatComp->GetAttackRange())
        {
            bNeedNewTarget = false;
        }
    }

    // 2. 새 타겟 필요 시 탐색
    if (bNeedNewTarget)
    {
        CurrentTarget = nullptr;
        ObjectStatComp->ResetHeatingStack(); // 타겟 잃으면 가열 스택 초기화
        SearchTarget();
    }

    // 3. 타겟이 확정되면 발사
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
    // 사거리 내 적 검색
    UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), ObjectStatComp->GetAttackRange(), 
        ObjectTypes, nullptr, IgnoreActors, OutActors);

    float ClosestDist = MAX_FLT;
    for (AActor* OverlapActor : OutActors)
    {
        ITargetable* TargetInterface = Cast<ITargetable>(OverlapActor);
        IDamageable* DamageInterface = Cast<IDamageable>(OverlapActor);

        if (TargetInterface && DamageInterface)
        {
            // 팀이 다르고 죽지 않은 적 중 가장 가까운 대상 선택
            if (TargetInterface->Execute_GetTeam(OverlapActor) != GetTeam() && !DamageInterface->Execute_IsDead(OverlapActor))
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
    if (!ProjectileClass || !::IsValid(CurrentTarget)) return;

    // 1. 투사체 소환 위치 및 방향 계산
    FVector SpawnLocation = FirePoint->GetComponentLocation();
    FRotator SpawnRotation = (CurrentTarget->GetActorLocation() - SpawnLocation).Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;

    if (ALoLRanged_Projectile* Projectile = GetWorld()->SpawnActor<ALoLRanged_Projectile>(ProjectileClass, SpawnLocation, SpawnRotation, Params))
    {
        // 2. 타겟 맞춤형 대미지 계산 호출
        float FinalCalculatedDamage = ObjectStatComp->GetAttackDamage(CurrentTarget.Get());
        
        // 3. 투사체 발사
        Projectile->Launch(CurrentTarget.Get(), 1500.f, FinalCalculatedDamage, GetTeam());

        // 4. [디테일] 챔피언 공격 시에만 가열 스택 관리
        ITargetable* TargetIT = Cast<ITargetable>(CurrentTarget.Get());
        if (TargetIT && TargetIT->Execute_GetUnitType(CurrentTarget.Get()) == EUnitType::Champion)
        {
            ObjectStatComp->AddHeatingStack();
    
            // Execute_ 대신 직접 public 함수 호출
            int32 CurrentStack = ObjectStatComp->GetCurrentHeatStack();
    
            PRINTLOG_HJ(TEXT("[포탑] 챔피언 공격! 가열 스택: %d | 대미지: %.1f"), CurrentStack, FinalCalculatedDamage);
        }
        else
        {
            // 타겟이 챔피언이 아니면 가열 스택 초기화
            ObjectStatComp->ResetHeatingStack();
        }
    }
}