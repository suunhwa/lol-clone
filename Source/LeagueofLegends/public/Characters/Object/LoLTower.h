#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLStructure.h"
#include "LoLTower.generated.h"

class ALoLRanged_Projectile;
class USceneComponent;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLTower : public ALoLStructure
{
	GENERATED_BODY()

public:
	ALoLTower();

protected:
	virtual void BeginPlay() override;

	/** 주기적으로 타겟 상태 확인 및 공격 시도 */
	void CheckAndAttack();

	/** 주변의 유효한 적 탐색 */
	void SearchTarget();

	/** 투사체 발사 및 대미지 처리 */
	void Fire();

	/** * 현재 타겟 (인터페이스 포인터가 아닌 AActor 포인터를 사용해야 안전함)
	 */
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<AActor> CurrentTarget;

	/** 투사체가 생성될 위치 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FirePoint;

	/** 에디터에서 할당할 투사체 클래스 (BP_LoLProjectile_Tower 등) */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<ALoLRanged_Projectile> ProjectileClass;

	FTimerHandle AttackTimerHandle;
};