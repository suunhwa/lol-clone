#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Siege.generated.h"

class ALoLRanged_Projectile;

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Siege : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Siege();

protected:
	// 대포 전용 원거리 공격 로직
	void ExecuteRangedAttack(AActor* Target);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ALoLRanged_Projectile> ProjectileClass;
};