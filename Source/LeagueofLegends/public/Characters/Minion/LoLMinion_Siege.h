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

	virtual void ExecuteAttack() override;

protected:
	// 법사(Ranged)와 동일한 투사체 발사 로직
	void ExecuteSiegeRangedAttack(AActor* Target);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ALoLRanged_Projectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileSpawnPoint;
};