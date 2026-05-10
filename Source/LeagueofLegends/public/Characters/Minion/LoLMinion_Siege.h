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

	// [성공 포인트] 법사와 동일하게 오버라이드 선언
	virtual void ExecuteAttack() override;

protected:
	// 직접 발사할 함수
	void ExecuteSiegeRangedAttack(AActor* Target);

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<ALoLRanged_Projectile> ProjectileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USceneComponent* ProjectileSpawnPoint;
};