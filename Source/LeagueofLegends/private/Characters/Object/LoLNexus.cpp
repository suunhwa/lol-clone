#include "Characters/Object/LoLNexus.h"

#include "LeagueofLegends.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/RiftGameState.h"

ALoLNexus::ALoLNexus()
{
	ObjectID = 11111; // 넥서스 고유 ID
}

void ALoLNexus::OnDestroyed()
{
    // 중복 실행 방지
    if (bIsDestroyed) return;
    bIsDestroyed = true;

    PRINTLOG_HJ(TEXT("[%s] 넥서스 파괴 시퀀스 시작! (부모 에셋 활용)"), *GetName());

    // 1. 사거리 데칼 및 UI 즉시 정리
    GetWorldTimerManager().ClearTimer(ProximityTimerHandle);
    if (RangeIndicatorDecal) RangeIndicatorDecal->SetVisibility(false);
    if (HPBarWidgetComp) HPBarWidgetComp->SetVisibility(false);
    
    // 타게팅 및 충돌 제외 처리
    SetActorEnableCollision(false);
    Tags.Empty();

    // 2. 부모의 연출용 변수(BreakingMesh, BreakAnim)를 그대로 활용해 파괴 애니메이션 재생
    float DelayTime = 1.5f; // 에셋 누락 시 기본 대기 시간

    if (MeshComp && BreakingMesh && BreakAnim)
    {
        // 부모 포탑 코드와 동일한 안정적인 메쉬 교체 프로세스
        MeshComp->UnregisterComponent(); 
        MeshComp->SetSkeletalMesh(BreakingMesh, true); 
        MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        MeshComp->RegisterComponent();

        // 넥서스가 부서지는 연출이므로 역재생하지 않고 '정방향(1.0f)'으로 재생하여 무너지게 합니다.
        float AnimPlayRate = 1.0f; 
        MeshComp->PlayAnimation(BreakAnim, false);
        MeshComp->SetPlayRate(AnimPlayRate);
        MeshComp->SetPosition(0.0f);
        
        MeshComp->RefreshBoneTransforms();
        MeshComp->UpdateComponentToWorld();

        // 애니메이션이 완전히 끝날 때까지 걸리는 정확한 시간 계산
        DelayTime = BreakAnim->GetPlayLength() / AnimPlayRate;
    }

    // 3. 맵 상태 갱신 신호는 보내주기 (주변 오브젝트 무적 해제 델리게이트가 있다면 작동)
    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        GS->BroadcastStructureStateChanged();
    }

    // 4. 애니메이션이 끝나는 타이밍에 화려하게 터지면서 슬로우 모션 걸기
    GetWorldTimerManager().SetTimer(GameOverTimerHandle, this, &ALoLNexus::HandleGameOver, DelayTime, false);
}

void ALoLNexus::HandleGameOver()
{
    // [연출 1] 애니메이션 최종 프레임 포즈로 멈춰 세우기 (박살난 상태 고정)
    if (MeshComp)
    {
        MeshComp->Stop();
    }

    // [연출 2] 부모에 선언된 대형 나이아가라 폭발 이펙트(ExplosionEffect) 펑!!
    if (ExplosionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), 
            ExplosionEffect, 
            GetActorLocation() + FVector(0, 0, 100.f), // 넥서스 정중앙 살짝 위
            GetActorRotation(),
            FVector(0.05f) // 넥서스는 대형 오브젝트니까 터지는 스케일을 웅장하게 키움
        );
        PRINTLOG_HJ(TEXT("[%s] 넥서스 최종 나이아가라 폭발 스폰 완료!"), *GetName());
    }
    else
    {
        // 폴백용 기본 이미터 
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), nullptr, GetActorLocation(), GetActorRotation(), FVector(5.0f));
    }

    // [연출 3] 롤 특유의 승리 직전 화면 흐름용 슬로우 모션 (0.2배속)
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);

    ETeam WinnerTeam = (GetTeam() == ETeam::Red) ? ETeam::Blue : ETeam::Red;
    FString WinnerString = (WinnerTeam == ETeam::Blue) ? TEXT("Blue") : TEXT("Red");
    PRINTLOG_HJ(TEXT("[게임 오버] %s 팀 승리! 슬로우 모션 연출 중..."), *WinnerString);

    // [연출 4] 완전히 무너진 잔해를 잠깐 감상하게 한 뒤 최종 UI 승패 판정 팝업
    FTimerHandle UIStopTimerHandle;
    GetWorldTimerManager().SetTimer(UIStopTimerHandle, [this, WinnerTeam]()
    {
        // TODO: 발표용 Victory / Defeat UI 위젯 오픈 한 줄 추가 위치
        // UIHUD->ShowGameResultWidget(WinnerTeam);
        
        PRINTLOG_HJ(TEXT("[최종 화면 정지] 결과 UI 오픈 완료."));
        // UGameplayStatics::SetGamePaused(GetWorld(), true); // 필요시 게임 서스펜드
    }, 2.0f, false); // 슬로우 모션이 걸린 상태라 체감상 아주 묵직하게 들어갑니다.
}