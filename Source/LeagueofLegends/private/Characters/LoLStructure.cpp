#include "Characters/LoLStructure.h"
#include "Manager/ObjectDataSubsystem.h" // 서브시스템 포함

ALoLStructure::ALoLStructure()
{
    PrimaryActorTick.bCanEverTick = false;
    ObjectID = 0; // 기본값
}

void ALoLStructure::BeginPlay()
{
    // 1. 데이터 테이블 로드 (부모 클래스 레벨에서 수행)
    InitializeStructureData();

    Super::BeginPlay();

    // 2. 태그 설정 (미니언/타워 타겟팅 시스템용)
    Tags.Empty();
    FName TeamTag = (Team == ETeam::Red) ? TEXT("RedTeam") : TEXT("BlueTeam");
    Tags.Add(TeamTag);
    Tags.Add(TEXT("Character")); // Character 태그 유지

    UE_LOG(LogTemp, Log, TEXT("[%s] 구조물 초기화 완료 - ID: %d, 팀: %s, 체력: %.1f"), 
        *GetName(), ObjectID, *TeamTag.ToString(), MaxHealth);
}

void ALoLStructure::InitializeStructureData()
{
    // 서브시스템 호출
    UObjectDataSubsystem* DataSubsystem = GetGameInstance()->GetSubsystem<UObjectDataSubsystem>();
    if (DataSubsystem)
    {
        // 서브시스템의 편의 함수를 이용해 3개 테이블 데이터를 한 번에 가져옴
        if (DataSubsystem->GetAllTowerData(ObjectID, StatData, RewardData, MechData))
        {
            // 가져온 데이터를 멤버 변수에 적용
            MaxHealth = StatData.Base_HP;
            Health = MaxHealth;
            HP_Regen = StatData.HP_Regen;
            
            // [로그] 데이터 연동 성공 알림
            UE_LOG(LogTemp, Warning, TEXT("========================================"));
            UE_LOG(LogTemp, Warning, TEXT("[Data Success] 이름: %s | ID: %d"), *GetName(), ObjectID);
            UE_LOG(LogTemp, Warning, TEXT("[Stats] HP: %.1f | AD: %.1f | Range: %.1f"), MaxHealth, StatData.Base_AD, StatData.Atk_Range);
            UE_LOG(LogTemp, Warning, TEXT("========================================"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[%s] ObjectID %d를 데이터 테이블에서 찾을 수 없습니다!"), *GetName(), ObjectID);
        }
    }
}

void ALoLStructure::ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator)
{
    if (bIsDestroyed) return;

    // 롤의 구조물은 보통 방어력이 적용되지만, 우선은 기본 로직 유지
    Health = FMath::Clamp(Health - Amount, 0.f, MaxHealth);
    
    UE_LOG(LogTemp, Log, TEXT("[%s] 구조물 피격! 남은 체력: %.1f"), *GetName(), Health);

    if (Health <= 0.f)
    {
       OnDestroyed();
    }
}

void ALoLStructure::OnDestroyed()
{
    if (bIsDestroyed) return;
    bIsDestroyed = true;

    // 1. 태그 제거 (타겟팅 방지)
    Tags.Empty();

    // 2. 파괴 보상 지급 (RewardData 활용)
    // 예: 플레이어 골드 지급 로직이 있다면 여기서 RewardData.Global_Gold 등을 사용
    
    UE_LOG(LogTemp, Error, TEXT("[%s] 구조물 파괴됨! 보상 골드: %.1f"), *GetName(), RewardData.Global_Gold);

    // 3. 시각 효과 및 소멸 처리
    // Destroy()는 보통 이펙트 후 처리를 위해 지연시키거나 BP에서 처리
}