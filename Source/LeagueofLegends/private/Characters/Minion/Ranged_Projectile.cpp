#include "Characters/Minion/Ranged_Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StatComponent.h"
#include "Components/TagComponent.h"
#include "Characters/LoLMinion.h"
#include "Kismet/GameplayStatics.h"

ALoLRanged_Projectile::ALoLRanged_Projectile()
{
    // 최적화를 위해 Tick은 끕니다 (ProjectileMovement가 대신 처리)
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile")); 
    RootComponent = CollisionComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->MaxSpeed = 7000.f; // 실제 탄속보다 넉넉하게 설정

    SetLifeSpan(3.0f); // 타겟에 못 부딪히면 3초 뒤 소멸
}

void ALoLRanged_Projectile::Launch(AActor* InTarget, float InSpeed, float InDamage, ETeam InOwnerTeam)
{
    TargetActor = InTarget;
    Damage = InDamage;
    OwnerTeam = InOwnerTeam;

    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = InSpeed;
        if (InTarget)
        {
            // 유도탄 설정 (Homing)
            ProjectileMovement->bIsHomingProjectile = true;
            ProjectileMovement->HomingTargetComponent = InTarget->GetRootComponent();
            // 탄속이 빠를수록 더 강한 힘으로 꺾어야 타겟을 놓치지 않음
            ProjectileMovement->HomingAccelerationMagnitude = InSpeed * 2.5f;
        }
        FVector Direction = InTarget ? (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal() : GetActorForwardVector();
        ProjectileMovement->Velocity = Direction * InSpeed;
    }
}

void ALoLRanged_Projectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    // 서버 권한 체크 및 자가 충돌 방지
    if (!HasAuthority() || !OtherActor || OtherActor == GetOwner()) return;

    // 타겟팅 투사체인 경우 지정된 타겟이 아니면 통과 (미니언 평타 특성)
    if (TargetActor.IsValid() && OtherActor != TargetActor.Get()) return;

    UTagComponent* OtherTag = OtherActor->FindComponentByClass<UTagComponent>();
    if (OtherTag)
    {
        // 적군인지 확인 (서로 다른 팀일 때만)
        if (OwnerTeam != ETeam::None && OtherTag->GetTeam() != OwnerTeam)
        {
            if (UStatComponent* TargetStat = OtherActor->FindComponentByClass<UStatComponent>())
            {
                // 대미지 적용
                UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), GetOwner(), UDamageType::StaticClass());
                
                // [로그 추가] 대미지 적용 확인
                PRINTLOG_HJ(TEXT("[투사체 충돌] %s -> %s 에게 %.1f 데미지 입힘!"), 
                    *GetOwner()->GetName(), *OtherActor->GetName(), Damage);
            }
            
            Destroy(); // 부딪히면 소멸
        }
    }
}