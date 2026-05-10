#include "Characters/LoLStructure.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "Manager/ObjectDataSubsystem.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Components/WidgetComponent.h"
#include "Manager/MinionDataSubsystem.h"

ALoLStructure::ALoLStructure()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // SkeletalMesh 컴포넌트 생성 및 루트 설정
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    
    ObjectStatComp = CreateDefaultSubobject<UObjectStatComponent>(TEXT("ObjectStatComp"));
    TagComp = CreateDefaultSubobject<UTagComponent>(TEXT("TagComp"));
    
    HPBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
    HPBarWidgetComp->SetupAttachment(RootComponent);
    HPBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen); 
    HPBarWidgetComp->SetDrawAtDesiredSize(true);
    
}

void ALoLStructure::BeginPlay()
{
    InitializeStructureData();
    Super::BeginPlay();
    // StatComponent의 OnHPChanged 델리게이트와 위젯 업데이트 함수 연결
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
    ObjectStatComp->ApplyDamage(Amount);
    if (ObjectStatComp->IsDead()) OnDestroyed();
}

bool ALoLStructure::IsDead_Implementation() const { return ObjectStatComp->IsDead(); }

ETeam ALoLStructure::GetTeam_Implementation() const { return TagComp->GetTeam(); }

EUnitType ALoLStructure::GetUnitType_Implementation() const
{
    return TagComp->GetUnitType();
}

void ALoLStructure::OnDestroyed()
{
    if (bIsDestroyed) return;
    bIsDestroyed = true;
    Tags.Empty();
}
