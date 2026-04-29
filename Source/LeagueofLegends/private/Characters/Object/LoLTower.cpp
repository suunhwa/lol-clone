
#include "Characters/Object/LoLTower.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ALoLTower::ALoLTower()
{
    // 기본 체력 설정
    Health = 2000.f;
    MaxHealth = 2000.f;
}

void ALoLTower::BeginPlay()
{
    Super::BeginPlay();

    // 1.2초마다 주변에 적이 있는지 확인하고 공격하는 타이머 시작
    GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ALoLTower::CheckAndAttack, AttackInterval, true);
}

void ALoLTower::CheckAndAttack()
{
    // 이미 타겟이 있다면, 그 타겟이 아직 유효한지(살아있는지, 범위 내에 있는지) 먼저 체크
    if (CurrentTarget)
    {
        float Dist = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
        IDamageable* Damageable = Cast<IDamageable>(CurrentTarget);
        
        if (Dist > AttackRange || (Damageable && Damageable->IsDead()))
        {
            CurrentTarget = nullptr; // 타겟 유실
        }
    }

    // 타겟이 없다면 새로 찾기
    if (!CurrentTarget)
    {
        TArray<AActor*> FoundActors;
        // 미니언과 마찬가지로 'Character' 태그가 달린 액터들을 찾음
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("Character"), FoundActors);

        float ClosestDist = AttackRange;
        FName MyTeamTag = (Tags.Num() > 0) ? Tags[0] : NAME_None;

        for (AActor* Actor : FoundActors)
        {
            if (!Actor || Actor == this) continue;

            // 적군 태그 확인
            if (!Actor->ActorHasTag(MyTeamTag))
            {
                float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
                if (Dist <= ClosestDist)
                {
                    ClosestDist = Dist;
                    CurrentTarget = Actor;
                }
            }
        }
    }

    // 최종적으로 타겟이 확정되었다면 발사!
    if (CurrentTarget)
    {
        Fire();
    }
}

void ALoLTower::Fire()
{
    if (!CurrentTarget) return;

    // [시각화] 타워에서 타겟으로 빨간 선을 그림 (투사체 대신)
    DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0,0,200), CurrentTarget->GetActorLocation(), FColor::Red, false, 0.2f, 0, 5.0f);

    // 데미지 입히기
    IDamageable* DamageableTarget = Cast<IDamageable>(CurrentTarget);
    if (DamageableTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Tower] %s를 공격합니다! 데미지: %.1f"), *CurrentTarget->GetName(), AttackDamage);
        DamageableTarget->ReceiveDamage(AttackDamage, EDamageType::Physical, this);
    }
}