#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Super.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Super : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Super();

	// 전사 미니언과 동일한 오버라이드 구조
	virtual void ExecuteAttack() override;
	virtual void BeginPlay() override;
};