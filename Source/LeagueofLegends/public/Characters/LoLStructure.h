#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Struct/ObjectStruct.h" // 구조체 포함 필수
#include "LoLStructure.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLStructure : public AActor, public IDamageable, public ITargetable
{
	GENERATED_BODY()

public:
	ALoLStructure();

protected:
	virtual void BeginPlay() override;

	/** 데이터 테이블로부터 스탯을 로드하는 함수 */
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
	// --- 핵심 설정 및 데이터 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Config")
	int32 ObjectID; // 데이터 테이블과 매칭될 ID (예: 101, 201 등)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Config")
	ETeam Team;

	// --- 런타임 스탯 (데이터 테이블에서 채워짐) ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object|Stats")
	float HP_Regen;

	// 데이터 보관용 (자식 클래스인 Tower 등에서 사용 가능하도록 protected)
	FObjectBaseRow StatData;
	FObjectRewardRow RewardData;
	FObjectMechanicsRow MechData;

	bool bIsDestroyed = false;

	virtual void OnDestroyed();
};