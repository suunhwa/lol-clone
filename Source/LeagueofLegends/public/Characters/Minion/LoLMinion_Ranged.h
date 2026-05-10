#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Ranged.generated.h"

// 전방 선언을 통해 의존성 최소화
class ALoLRanged_Projectile;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Ranged : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Ranged();

	
	virtual void ExecuteAttack() override;
	
protected:
	// 원거리 공격 실행 함수
	void ExecuteRangedAttack(AActor* Target);

	// 에디터에서 ALoLRanged_Projectile 기반 블루프린트를 할당
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ALoLRanged_Projectile> ProjectileClass;
};