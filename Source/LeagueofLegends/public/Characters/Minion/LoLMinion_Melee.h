#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Melee.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Melee : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Melee();

protected:
	// 부모의 비어있던 공격 함수를 전사 미니언에 맞게 재정의합니다.
	virtual void PerformAttack() override;
};