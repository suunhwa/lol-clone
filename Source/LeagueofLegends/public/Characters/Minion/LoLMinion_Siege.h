#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Siege.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Siege : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Siege();

protected:
	// 원거리 공격 로직 구현
	virtual void PerformAttack() override;

	// 투사체 클래스 설정 (Ranged_Projectile 재사용)
	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	TSubclassOf<class ALoLRanged_Projectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	FVector ProjectileOffset = FVector(80.f, 0.f, 100.f);
};