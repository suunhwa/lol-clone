#pragma once
#include "Characters/LoLStructure.h"
#include "LoLTower.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLTower : public ALoLStructure
{
	GENERATED_BODY()
public:
	ALoLTower();
protected:
	virtual void BeginPlay() override;
	void CheckAndAttack();
	void Fire();

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
	FTimerHandle AttackTimerHandle;
};