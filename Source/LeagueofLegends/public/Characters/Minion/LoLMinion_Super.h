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

protected:
	virtual void PerformAttack() override;
};