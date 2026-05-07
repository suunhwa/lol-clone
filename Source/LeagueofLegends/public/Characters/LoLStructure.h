#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Struct/ObjectStruct.h"
#include "LoLStructure.generated.h"

class UObjectStatComponent;
class UTagComponent;
class USkeletalMeshComponent;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLStructure : public AActor, public IDamageable, public ITargetable
{
    GENERATED_BODY()

public:
    ALoLStructure();

protected:
    virtual void BeginPlay() override;
    virtual void InitializeStructureData();
    virtual void OnDestroyed();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UObjectStatComponent> ObjectStatComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTagComponent> TagComp;

/*public:
    // --- IDamageable Interface ---
    virtual void ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
    virtual bool IsDead_Implementation() const override { return bIsDestroyed; }

    // --- ITargetable Interface ---
    virtual bool IsTargetable_Implementation() const override { return !bIsDestroyed; }
    virtual FVector GetTargetLocation_Implementation() const override { return GetActorLocation(); }
    virtual ETeam GetTeam_Implementation() const override { return Team; }
*/

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
    
public:
    virtual void ReceiveDamage_Implementation(float Amount, EDamageType DamageType, AActor* DamageInstigator) override;
    virtual bool IsDead_Implementation() const override;
    virtual bool IsTargetable_Implementation() const override { return !IsDead(); }
    virtual ETeam GetTeam_Implementation() const override;
    virtual FVector GetTargetLocation_Implementation() const override { return GetActorLocation(); }
};