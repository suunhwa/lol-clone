#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Struct/ObjectStruct.h"
#include "LoLStructure.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLStructure : public AActor, public IDamageable, public ITargetable
{
    GENERATED_BODY()

public:
    ALoLStructure();

protected:
    virtual void BeginPlay() override;

    /** 데이터 테이블로부터 모든 스탯을 멤버 변수에 할당하는 함수 */
    virtual void InitializeStructureData();

public:
    // --- IDamageable Interface ---
    virtual void ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
    virtual bool IsDead() const override { return bIsDestroyed; }

    // --- ITargetable Interface ---
    virtual bool IsTargetable() const override { return !bIsDestroyed; }
    virtual FVector GetTargetLocation() const override { return GetActorLocation(); }
    virtual ETeam GetTeam() const override { return Team; }

protected:
    // --- 핵심 설정 ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Config")
    int32 ObjectID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Config")
    ETeam Team;

    // --- [Base Stats] ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    FString ObjectType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float HP_Regen;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float AttackDamage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float AD_Growth_Per_Min;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float Max_AD;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float AttackRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
    float AttackSpeed;

    // --- [Reward Stats] ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    float Plate_Gold;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    float Total_Plates;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    float Global_Gold;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    float Last_Hit_Gold;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    float Global_Exp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    bool bIncludeDead;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    bool bGlobalDist;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Reward")
    float Exp_Range;

    // --- [Mechanics Stats] ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    float Heating_Rate;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    int32 Max_Heating;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    float Plate_Expiry_Time;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    float CurrentArmor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    float Plate_Armor_Bonus;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    int32 Spawn_Unit_ID;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Mechanics")
    float Respawn_Time;

    // --- 내부 상태 변수 ---
    FObjectBaseRow StatData;
    FObjectRewardRow RewardData;
    FObjectMechanicsRow MechData;

    bool bIsDestroyed = false;

    virtual void OnDestroyed();
};