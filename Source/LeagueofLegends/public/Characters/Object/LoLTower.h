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

	// 주기적으로 실행될 로직
	void CheckAndAttack();

	// 타겟을 찾는 로직
	void SearchTarget();

	// 실제 투사체 발사
	void Fire();

	// 현재 타겟
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<AActor> CurrentTarget;

	// 투사체 소환 지점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> FirePoint;

	// 에디터에서 할당할 투사체 클래스 (법사 미니언과 동일한 것 사용 가능)
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<ALoLRanged_Projectile> ProjectileClass;

	FTimerHandle AttackTimerHandle;
};