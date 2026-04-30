#include "Characters/LoLStructure.h"
#include "Manager/ObjectDataSubsystem.h"

ALoLStructure::ALoLStructure()
{
    PrimaryActorTick.bCanEverTick = false;
    ObjectID = 0;
    Team = ETeam::None;
    bIsDestroyed = false;
}

void ALoLStructure::BeginPlay()
{
    // 1. 데이터 테이블 로드 및 모든 변수 할당
    InitializeStructureData();

    Super::BeginPlay();

    // 2. 태그 설정
    FName TeamTag = (Team == ETeam::Red) ? TEXT("RedTeam") : TEXT("BlueTeam");
    Tags.Add(TeamTag);
    Tags.Add(TEXT("Character")); 

    UE_LOG(LogTemp, Log, TEXT("[%s] 구조물 풀 스탯 초기화 완료 - ID: %d, 팀: %s"), 
        *GetName(), ObjectID, *TeamTag.ToString());
}

void ALoLStructure::InitializeStructureData()
{
    UObjectDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UObjectDataSubsystem>();
    if (DataSubsystem)
    {
        if (DataSubsystem->GetAllTowerData(ObjectID, StatData, RewardData, MechData))
        {
            // [Base Section]
            ObjectType = StatData.Object_Type;
            MaxHealth = StatData.Base_HP;
            Health = MaxHealth;
            HP_Regen = StatData.HP_Regen;
            AttackDamage = StatData.Base_AD;
            AD_Growth_Per_Min = StatData.AD_Growth_Per_Min;
            Max_AD = StatData.Max_AD;
            AttackRange = StatData.Atk_Range;
            AttackSpeed = StatData.Atk_Speed;

            // [Reward Section]
            Plate_Gold = RewardData.Plate_Gold;
            Total_Plates = RewardData.Total_Plates;
            Global_Gold = RewardData.Global_Gold;
            Last_Hit_Gold = RewardData.Last_Hit_Gold;
            Global_Exp = RewardData.Global_Exp;
            bIncludeDead = RewardData.bIncludeDead;
            bGlobalDist = RewardData.bGlobalDist;
            Exp_Range = RewardData.Exp_Range;

            // [Mechanics Section]
            Heating_Rate = MechData.Heating_Rate;
            Max_Heating = MechData.Max_Heating;
            Plate_Expiry_Time = MechData.Plate_Expiry_Time;
            CurrentArmor = MechData.Base_Armor_After_Expiry; // 초기 방어력 설정
            Plate_Armor_Bonus = MechData.Plate_Armor_Bonus;
            Spawn_Unit_ID = MechData.Spawn_Unit_ID;
            Respawn_Time = MechData.Respawn_Time;

            UE_LOG(LogTemp, Warning, TEXT("========================================"));
            UE_LOG(LogTemp, Warning, TEXT("[Data Full Load Success] %s (ID: %d)"), *GetName(), ObjectID);
            UE_LOG(LogTemp, Warning, TEXT("HP: %.1f | AD: %.1f | Armor: %.1f | Reward: %.1f Gold"), 
                MaxHealth, AttackDamage, CurrentArmor, Global_Gold);
            UE_LOG(LogTemp, Warning, TEXT("========================================"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] ObjectID %d를 찾을 수 없어 데이터 연동에 실패했습니다."), *GetName(), ObjectID);
        }
    }
}

void ALoLStructure::ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
    if (bIsDestroyed) return;

    // 롤 방어력 공식: 실제 피해량 = 데미지 * (100 / (100 + 방어력))
    float DamageMultiplier = 100.0f / (100.0f + CurrentArmor);
    float FinalDamage = Amount * DamageMultiplier;

    Health = FMath::Clamp(Health - FinalDamage, 0.f, MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("[%s] ReceiveDamage - 원본: %.1f, 최종: %.1f, 남은 HP: %.1f"), 
        *GetName(), Amount, FinalDamage, Health);
    
    if (Health <= 0.f)
    {
       OnDestroyed();
    }
}

void ALoLStructure::OnDestroyed()
{
    if (bIsDestroyed) return;
    bIsDestroyed = true;

    Tags.Empty();

    // 파괴 시 골드 보상 로직 (예시: 모든 플레이어에게 Global_Gold 지급 등)
    UE_LOG(LogTemp, Error, TEXT("[%s] 구조물 파괴! 전역 골드 지급: %.1f, 막타 골드: %.1f"), 
        *GetName(), Global_Gold, Last_Hit_Gold);

    // TODO: 시각 효과(파괴 애니메이션) 및 실제 Destroy 처리 로직 추가
}