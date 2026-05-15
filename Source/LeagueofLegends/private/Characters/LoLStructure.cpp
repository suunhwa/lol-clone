#include "Characters/LoLStructure.h"

#include "LeagueofLegends.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "Manager/ObjectDataSubsystem.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Components/WidgetComponent.h"
#include "FOW/FOWManager.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/MinionDataSubsystem.h"
#include "UI/View/HPBarWidget.h"

ALoLStructure::ALoLStructure()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // SkeletalMesh 컴포넌트 생성 및 루트 설정
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    SetRootComponent(MeshComp);
    
    // 캐릭터가 못 지나가게 막음
    MeshComp->SetCollisionProfileName(TEXT("BlockAll")); // 모든 것을 막음 (벽처럼 작동)
    MeshComp->SetCanEverAffectNavigation(true);
    
    // 타게팅 탐색을 위해 필요
    MeshComp->SetGenerateOverlapEvents(true);
    
    ObjectStatComp = CreateDefaultSubobject<UObjectStatComponent>(TEXT("ObjectStatComp"));
    TagComp = CreateDefaultSubobject<UTagComponent>(TEXT("TagComp"));
    
    HPBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
    HPBarWidgetComp->SetupAttachment(RootComponent);
    HPBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); 
    HPBarWidgetComp->SetDrawAtDesiredSize(true);
}

ERiftSightTag ALoLStructure::GetSightTag_Implementation() const
{
    // Actor의 이름과 GetSightTag을 함께 출력해서 디버깅하기
    return TagComp->GetSightTag();
}

void ALoLStructure::BeginPlay()
{
    Super::BeginPlay();
    InitializeStructureData();
    // StatComponent의 OnHPChanged 델리게이트와 위젯 업데이트 함수 연결
    if (HPBarWidgetComp)
    {
        if (auto* HPWidget = Cast<UHPBarWidget>(HPBarWidgetComp->GetUserWidgetObject()))
        {
            // 이제 타입 걱정 없이 포탑(this)을 넘겨주면 됨!
            HPWidget->InitWidgetFromDamageable(this);
        }
    }
    
    if (ObjectStatComp)
    {
        ObjectStatComp->OnHPChanged.AddUObject(this, &ALoLStructure::UpdateHPBar);
    }
    
    if (TagComp)
    {
        TagComp->SetTeam(InitialTeam);
        Tags.Add((InitialTeam == ETeam::Red) ? TEXT("RedTeam") : TEXT("BlueTeam"));
        Tags.Add(TEXT("Structure"));
    }
    
    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        if (AFOWManager* FOWManager = GS->GetFOWManager())
        {
            FOWManager->RegisterSightProvider(this);
        }
        else
        {
            PRINTLOG_TK(TEXT("RegisterSightProvider failed: FOWManager is null for %s"), *GetName());
        }
    }
    else
    {
        PRINTLOG_TK(TEXT("RegisterSightProvider failed: GameState is null for %s"), *GetName());
    }
    
    GetWorldTimerManager().SetTimerForNextTick(this, &ALoLStructure::RefreshVulnerability);
    
    // GameState의 이벤트에 구독 (부활/파괴 시 신호를 받음)
    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        GS->OnStructureStateChanged.AddDynamic(this, &ALoLStructure::RefreshVulnerability);
    }
    
    /*// 게임 시작 시 초기 상태 설정
    RefreshVulnerability();*/
}

void ALoLStructure::InitializeStructureData()
{
    UObjectDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UObjectDataSubsystem>();
    if (DataSubsystem && DataSubsystem->GetAllTowerData(ObjectID, StatData, RewardData, MechData))
    {
        ObjectStatComp->InitObjectStats(StatData, RewardData, MechData);
    }
}

void ALoLStructure::UpdateHPBar(float CurrentHP, float MaxHP)
{
    if (HPBarWidgetComp)
    {
        // 1. 위젯 클래스 가져오기 (형진이가 만든 위젯 클래스로 캐스팅)
        UUserWidget* HPWidget = HPBarWidgetComp->GetUserWidgetObject();
        if (HPWidget)
        {
            // 2. 블루프린트에서 만든 'UpdateHP' 같은 함수를 호출하거나, 
            // 직접 프로그레스 바 변수에 접근해서 Percent를 설정해줘.
            // (가장 쉬운 방법은 블루프린트에서 이벤트를 받는 거야)
            ReceiveUpdateHP(CurrentHP, MaxHP); 
        }
    }
}

void ALoLStructure::ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
    if (bIsDestroyed || !ObjectStatComp) return;
    
    // 무적 상태라면 대미지 무시
    if (!bIsVulnerable)
    {
        PRINTLOG_HJ(TEXT("[%d] 무적 상태! 상위 오브젝트를 먼저 파괴하세요."), ObjectID);
        return;
    }
    
    PRINTLOG_HJ(TEXT("[포탑 피격] %s로부터 %.1f 대미지 전달됨!"), DamageInstigator ? *DamageInstigator->GetName() : TEXT("Unknown"), Amount);
    ObjectStatComp->ApplyDamage(Amount);
    if (ObjectStatComp->IsDead()) OnDestroyed();
}

bool ALoLStructure::IsDead_Implementation() const { return ObjectStatComp->IsDead(); }

ETeam ALoLStructure::GetTeam_Implementation() const { return TagComp->GetTeam(); }

EUnitType ALoLStructure::GetUnitType_Implementation() const
{
    return TagComp->GetUnitType();
}

void ALoLStructure::RefreshVulnerability()
{
    if (bIsDestroyed) return;

    // 1. [핵심 방어] 부모 리스트가 비어 있는 경우
    if (ParentActors.Num() == 0)
    {
        // ObjectID가 11001(1차 타워)인 경우에만 시작부터 열려있어야 합니다.
        // 만약 2차(11002)나 3차(11003)인데 부모가 없다면 설정 오류이므로 닫아버립니다.
        if (ObjectID == 11001)
        {
            bIsVulnerable = true;
        }
        else
        {
            // 에디터에서 스포이드로 부모를 안 찍었을 때 로그를 남겨줍니다.
            PRINTLOG_HJ(TEXT("[%d] 경고: 1차 타워가 아닌데 부모가 없습니다! 무적을 유지합니다."), ObjectID);
            bIsVulnerable = false;
        }
        return;
    }

    bool bNewVulnerable = false;

    // 2. 상위 오브젝트 파괴 여부 판단
    if (bRequireAllParentsDead)
    {
        // [AND 조건] 모든 부모가 죽어야 함 (보통 2차, 3차 타워)
        bool bAllDead = true;
        for (const TSoftObjectPtr<ALoLStructure>& ParentPtr : ParentActors)
        {
            // 소프트 포인터를 동기식으로 로드하여 확인
            ALoLStructure* Parent = ParentPtr.LoadSynchronous();
            
            // 부모를 못 찾거나, 부모가 아직 살아있다면 false
            if (!Parent || !IDamageable::Execute_IsDead(Parent))
            {
                bAllDead = false;
                break;
            }
        }
        bNewVulnerable = bAllDead;
    }
    else
    {
        // [OR 조건] 하나라도 죽으면 해제 (넥서스 타워용)
        bool bAnyDead = false;
        for (const TSoftObjectPtr<ALoLStructure>& ParentPtr : ParentActors)
        {
            ALoLStructure* Parent = ParentPtr.LoadSynchronous();
            if (Parent && IDamageable::Execute_IsDead(Parent))
            {
                bAnyDead = true;
                break;
            }
        }
        bNewVulnerable = bAnyDead;
    }

    // 3. 상태가 바뀔 때만 로그를 찍고 변수 업데이트
    if (bIsVulnerable != bNewVulnerable)
    {
        bIsVulnerable = bNewVulnerable;
        if (bIsVulnerable)
        {
            PRINTLOG_HJ(TEXT("[%d] 타워 무적 해제! 공격 가능."), ObjectID);
        }
    }
}

void ALoLStructure::OnDestroyed()
{
    if (bIsDestroyed) return;
    bIsDestroyed = true;
    
    // 1. UI 및 시야 정리
    if (HPBarWidgetComp) HPBarWidgetComp->SetVisibility(false);
    Tags.Empty();

    // 2. 부숴지는 연출 단계 (두 번째 뼈대)
    if (MeshComp && BreakingMesh && BreakAnim)
    {
        // [핵심] 뼈대가 다르므로 컴포넌트를 아예 '미등록 -> 메쉬교체 -> 재등록' 합니다.
        // 이렇게 해야 언리얼이 새로운 뼈대 구조(Skeleton)를 완벽하게 인지합니다.
        MeshComp->UnregisterComponent(); 
        
        // bReinitPose를 true로 주어 이전 뼈대 흔적을 지웁니다.
        MeshComp->SetSkeletalMesh(BreakingMesh, true); 
        
        // 애니메이션 모드와 틱 옵션 강제 설정
        MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        
        // 컴포넌트 재등록
        MeshComp->RegisterComponent();

        // [재생] 이제 새로운 뼈대 위에서 애니메이션을 돌립니다.
        MeshComp->PlayAnimation(BreakAnim, false);
        
        // 즉시 렌더링 업데이트
        MeshComp->SetPosition(0.0f);
        MeshComp->RefreshBoneTransforms();
        MeshComp->UpdateComponentToWorld();
        // 역재생
        MeshComp->SetPlayRate(-1.0f); // 재생 속도를 -1로 설정 (역재생)
        MeshComp->SetPosition(BreakAnim->GetPlayLength());
        
        float AnimDuration = BreakAnim->GetPlayLength();
        PRINTLOG_HJ(TEXT("[%s] 연출 메쉬 교체 및 애니메이션 재생 시작"), *GetName());

        // 3. 마지막 단계: 파괴된 잔해 고정 (세 번째 뼈대)
        GetWorldTimerManager().SetTimer(MeshSwapTimerHandle, [this]()
        {
            if (MeshComp && DestroyedMesh)
            {
                // 잔해 메쉬도 뼈대가 다르므로 다시 한번 재등록 절차
                MeshComp->UnregisterComponent();
                MeshComp->SetSkeletalMesh(DestroyedMesh, true);
                MeshComp->RegisterComponent();
                
                MeshComp->Stop(); // 잔해는 움직이지 않음
                PRINTLOG_HJ(TEXT("[%s] 최종 잔해 메쉬 고정 완료"), *GetName());
            }
        }, AnimDuration, false);
    }
    else if (MeshComp && DestroyedMesh)
    {
        // 연출용 에셋이 없을 경우 바로 잔해로 점프
        MeshComp->SetSkeletalMesh(DestroyedMesh, true);
    }

    // 게임 상태 업데이트 (무적 해제 등)
    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        GS->BroadcastStructureStateChanged();
    }
}