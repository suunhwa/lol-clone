#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/SightProvider.h"
#include "Interfaces/Targetable.h"
#include "Struct/ObjectStruct.h"
#include "LoLStructure.generated.h"

class UObjectStatComponent;
class UTagComponent;
class USkeletalMeshComponent;
class UNiagaraSystem;
class UDecalComponent;


UCLASS()
class LEAGUEOFLEGENDS_API ALoLStructure : public AActor, public IDamageable, public ITargetable, public ISightProvider
{
    GENERATED_BODY()

public:
    ALoLStructure();

    // --- ISightProvider Interface 구현 ---
    virtual FVector GetSightOrigin_Implementation() const override { return GetActorLocation(); }
    virtual float GetSightRange_Implementation() const override { return 1200; }
    virtual bool IsStatic_Implementation() const override { return true; }
    virtual ERiftSightTag GetSightTag_Implementation() const override;
    virtual bool IsHideable_Implementation() const override { return false; }
    
protected:
    virtual void BeginPlay() override;
    virtual void InitializeStructureData();
    virtual void OnDestroyed();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UObjectStatComponent> ObjectStatComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTagComponent> TagComp;
    
    UPROPERTY(EditAnywhere, Category = "Config")
    int32 ObjectID;

    UPROPERTY(EditAnywhere, Category = "Config")
    ETeam InitialTeam;

    FObjectBaseRow StatData;
    FObjectRewardRow RewardData;
    FObjectMechanicsRow MechData;

    bool bIsDestroyed = false;

    // SkeletalMesh 컴포넌트 추가
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> MeshComp;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UWidgetComponent> HPBarWidgetComp;

    void UpdateHPBar(float CurrentHP, float MaxHP);
    
    UFUNCTION(BlueprintImplementableEvent)
    void ReceiveUpdateHP(float CurrentHP, float MaxHP);
    
public:
    virtual void ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
    virtual bool IsDead_Implementation() const override;
    virtual bool IsTargetable_Implementation() const override { return !IsDead(); }
    virtual ETeam GetTeam_Implementation() const override;
    virtual FVector GetTargetLocation_Implementation() const override { return GetActorLocation(); }
    virtual EUnitType GetUnitType_Implementation() const override;
    virtual AActor* GetCurrentCombatTarget_Implementation() const override { return nullptr; }
    
    
    
    // [에디터 설정] 이 타워가 공격받기 위해 먼저 파괴되어야 하는 실제 타워 액터들
    UPROPERTY(EditAnywhere, Category = "Config | Sequence")
    TArray<TSoftObjectPtr<ALoLStructure>> ParentActors;

    // True(AND): 모든 부모가 파괴되어야 함 (일반 타워용)
    // False(OR): 부모 중 하나라도 파괴되면 됨 (넥서스 타워용)
    UPROPERTY(EditAnywhere, Category = "Config | Sequence")
    bool bRequireAllParentsDead = true;

    // 현재 공격 가능한 상태인지 저장
    UPROPERTY(VisibleInstanceOnly, Category = "Status")
    bool bIsVulnerable = false;

    // 무적 상태 업데이트 함수 (UFUNCTION을 붙여야 dynamic 델리게이트 바인딩이 가능합니다)
    UFUNCTION()
    void RefreshVulnerability();
    
    
protected:
    // --- 메쉬 돌려막기용 변수 ---
    
    // 1. 평상시 메쉬 (LoLTurret_Blue)
    UPROPERTY(EditAnywhere, Category = "Design|Mesh")
    TObjectPtr<USkeletalMesh> NormalMesh;

    // 2. 파괴 연출용 메쉬 (LoLTurret_Blue_Respawn - 역재생으로 사용)
    UPROPERTY(EditAnywhere, Category = "Design|Mesh")
    TObjectPtr<USkeletalMesh> BreakingMesh;

    // 3. 최종 파괴 메쉬 (LoLTurret_Blue_Destroy - 밑동만 남은 상태)
    UPROPERTY(EditAnywhere, Category = "Design|Mesh")
    TObjectPtr<USkeletalMesh> DestroyedMesh;

    // 파괴 시 재생할 애니메이션 (Respawn 파일에서 가져온 것)
    UPROPERTY(EditAnywhere, Category = "Design|Animation")
    TObjectPtr<UAnimSequence> BreakAnim;

    // 메쉬 교체용 타이머 핸들
    FTimerHandle MeshSwapTimerHandle;
    
    // 파괴 시 뿜어져 나올 나이아가라 시스템 (아까 만든 NS_Explosion)
    UPROPERTY(EditAnywhere, Category = "Design|Effect")
    TObjectPtr<UNiagaraSystem> ExplosionEffect;

    
protected:
    // 사거리 표시용 데칼 (바닥 링)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UDecalComponent> RangeIndicatorDecal;

    // 플레이어 감지용 함수 및 타이머
    void CheckPlayerProximity();
    FTimerHandle ProximityTimerHandle;
    
};