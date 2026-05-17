#include "Characters/Object/LoLInhibitor.h"

#include "LeagueofLegends.h"
#include "NiagaraFunctionLibrary.h"
#include "Characters/Minion/MinionSpawner.h"
#include "Components/DecalComponent.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"

ALoLInhibitor::ALoLInhibitor()
{
    ObjectID = 11101;
}

void ALoLInhibitor::OnDestroyed()
{
    if (bIsDestroyed) return;

    // 타이머 및 인디케이터 정리
    GetWorldTimerManager().ClearTimer(ProximityTimerHandle);
    if (RangeIndicatorDecal) RangeIndicatorDecal->SetVisibility(false);
    
    bIsDestroyed = true;
    
    if (HPBarWidgetComp) HPBarWidgetComp->SetVisibility(false);
    Tags.Empty();

    // 1. [수정] 2번째 메쉬(BreakingMesh)로 파괴 애니메이션 정재생 후 그 자리에서 멈춤(타임스톱)
    if (MeshComp && BreakingMesh && BreakAnim)
    {
        if (ExplosionEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), 
                ExplosionEffect, 
                GetActorLocation() + FVector(0, 0, 100), 
                GetActorRotation(),
                FVector(0.05f)
            );
        }
        
        // 메쉬 교체 절차 (파괴 연출용 메쉬로 변경)
        MeshComp->UnregisterComponent(); 
        MeshComp->SetSkeletalMesh(BreakingMesh, true); 
        MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        MeshComp->RegisterComponent();

        // Death 애니메이션을 정방향으로 재생
        MeshComp->PlayAnimation(BreakAnim, false);
        MeshComp->SetPlayRate(2.0f);   // 정재생 속도
        MeshComp->SetPosition(0.0f);   // 처음부터 시작해서 정상적으로 무너지게 함
        
        MeshComp->RefreshBoneTransforms();
        MeshComp->UpdateComponentToWorld();
        
        float AnimDuration = BreakAnim->GetPlayLength() / 2.0f; // 재생 속도(2.0) 고려한 실제 시간

        // [핵심 변경] 세 번째 메쉬로 바꾸지 않고, 스크린샷의 그 마지막 프레임 상태 그대로 일시정지!
        GetWorldTimerManager().SetTimer(MeshSwapTimerHandle, [this]()
        {
            if (MeshComp)
            {
                // 애니메이션을 멈춰서 완전히 부서진 잔해 상태 포즈로 고정시킵니다.
                MeshComp->Stop(); 
                PRINTLOG_HJ(TEXT("[%s] 억제기 파괴 애니메이션 최종 프레임 포즈 고정 완료 (타임스톱)"), *GetName());
            }
        }, AnimDuration, false);
    }
    else if (MeshComp && BreakingMesh) // 만약 에셋 누락 시 최소한 메쉬는 유지
    {
        MeshComp->SetSkeletalMesh(BreakingMesh, true);
    }

    // 2. 물리 충돌 비활성화 및 미니언 스포너 연동
    SetActorEnableCollision(false);
    UpdateSpawner(true);

    // 3. 리스폰 타이머 작동
    float RespawnTime = (MechData.Respawn_Time > 0.f) ? MechData.Respawn_Time : 300.f;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ALoLInhibitor::Respawn, RespawnTime, false);
    
    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        GS->BroadcastStructureStateChanged();
    }

    ETeam CurrentTeam = ITargetable::Execute_GetTeam(this);
    PRINTLOG_HJ(TEXT("[억제기] 파괴됨! %s팀의 슈퍼 미니언 소환 및 %f초 리스폰 타이머 시작."), 
        (CurrentTeam == ETeam::Red) ? TEXT("Blue") : TEXT("Red"), RespawnTime);
}

void ALoLInhibitor::Respawn()
{
    bIsDestroyed = false;
    SetActorEnableCollision(true);

    // 1. [수정] 멈춰있던 2번째 메쉬(BreakingMesh) 상태에서 그대로 역재생을 돌린 뒤, 완료되면 멀쩡한 메쉬로 교체
    if (MeshComp && BreakingMesh && NormalMesh && BreakAnim)
    {
       MeshComp->UnregisterComponent();
       MeshComp->SetSkeletalMesh(BreakingMesh, true); // 여전히 부서진 메쉬 상태를 유지한 채로 역재생해야 조립이 보입니다.
        
       MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
       MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
       MeshComp->RegisterComponent();
        
       // 완전히 무너진 끝 프레임(GetPlayLength)에서부터 거꾸로(-2.0f) 감아 올립니다.
       MeshComp->PlayAnimation(BreakAnim, false);
       MeshComp->SetPlayRate(-2.0f);                      // 역재생 속도
       MeshComp->SetPosition(BreakAnim->GetPlayLength()); // 애니메이션 맨 끝에서부터 역순 재생
        
       MeshComp->RefreshBoneTransforms();
       MeshComp->UpdateComponentToWorld();

       // 조립 애니메이션이 완료되는 시점(AnimDuration)에 멀쩡한 메쉬(NormalMesh)로 교체합니다.
       float AnimDuration = BreakAnim->GetPlayLength() / 2.0f;
       GetWorldTimerManager().SetTimer(MeshSwapTimerHandle, [this]()
       {
           if (MeshComp && NormalMesh)
           {
               MeshComp->UnregisterComponent();
               MeshComp->SetSkeletalMesh(NormalMesh, true); // 조립 완료되었으니 멀쩡한 평상시 메쉬로 교체!
               MeshComp->RegisterComponent();
               MeshComp->Stop(); // 정지 상태 유지
               PRINTLOG_HJ(TEXT("[%s] 억제기 부활 역재생 완료 -> 평상시 메쉬(NormalMesh)로 교체 고정"), *GetName());
           }
       }, AnimDuration, false);
    }

    // 2. 스탯 재초기화
    if (ObjectStatComp)
    {
       ObjectStatComp->InitObjectStats(StatData, RewardData, MechData);
    }
    
    // 3. HP 바 복구 및 풀피 세팅
    if (HPBarWidgetComp && ObjectStatComp)
    {
       HPBarWidgetComp->SetVisibility(true);
       float MaxHP = ObjectStatComp->GetMaxHP();
       UpdateHPBar(MaxHP, MaxHP);
    }

    // 4. 태그 복구
    if (TagComp)
    {
       Tags.Add((InitialTeam == ETeam::Red) ? TEXT("RedTeam") : TEXT("BlueTeam"));
       Tags.Add(TEXT("Structure"));
    }

    // 5. 일반 미니언 소환으로 복구
    UpdateSpawner(false);

    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
       GS->BroadcastStructureStateChanged();
    }
    
    PRINTLOG_HJ(TEXT("[억제기] 멈춰있던 파괴 포즈에서 역재생 부활 연출을 시작합니다."));
}

void ALoLInhibitor::UpdateSpawner(bool bSpawnSuper)
{
    // (기존 UpdateSpawner 로직과 동일하여 유지)
    UTagComponent* MyTag = FindComponentByClass<UTagComponent>();
    if (!MyTag) return;
    
    ETeam MyTeam = MyTag->GetTeam();

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMinionSpawner::StaticClass(), FoundActors);

    for (AActor* SpawnerActor : FoundActors)
    {
       AMinionSpawner* Spawner = Cast<AMinionSpawner>(SpawnerActor);
       if (Spawner)
       {
          UTagComponent* SpawnerTag = Spawner->FindComponentByClass<UTagComponent>();
          if (SpawnerTag)
          {
             if (SpawnerTag->GetTeam() != MyTeam)
             {
                Spawner->bIsInhibitorDestroyed = bSpawnSuper;

                PRINTLOG_HJ(TEXT("[억제기 연동] 상대 팀(%s) 스포너 발견! 슈퍼 미니언 모드: %s"), 
                   (SpawnerTag->GetTeam() == ETeam::Red) ? TEXT("Red") : TEXT("Blue"),
                   bSpawnSuper ? TEXT("ON") : TEXT("OFF"));
             }
          }
       }
    }
}