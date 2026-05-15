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
    
    PRINTLOG_HJ(TEXT("[%s] 포탑 파괴 로직 실행!"), *GetName());
    
    // 충돌 끄기 (더 이상 타겟팅 안 되게)
    SetActorEnableCollision(false);
    Tags.Empty();
    
    // 내가 파괴되었음을 알림 (다른 타워들의 RefreshVulnerability 유도)
    if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
    {
        GS->BroadcastStructureStateChanged();
    }
}
