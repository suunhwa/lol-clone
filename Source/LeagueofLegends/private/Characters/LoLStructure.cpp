#include "Characters/LoLStructure.h"
#include "Components/ObjectStatComponent.h"
#include "Components/TagComponent.h"
#include "Manager/ObjectDataSubsystem.h"
#include "Components/SkeletalMeshComponent.h" 

ALoLStructure::ALoLStructure()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // SkeletalMesh 컴포넌트 생성 및 루트 설정
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    
    ObjectStatComp = CreateDefaultSubobject<UObjectStatComponent>(TEXT("ObjectStatComp"));
    TagComp = CreateDefaultSubobject<UTagComponent>(TEXT("TagComp"));
}

void ALoLStructure::BeginPlay()
{
    InitializeStructureData();
    Super::BeginPlay();

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
