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

	// [성공 포인트] 전사와 동일하게 오버라이드 선언
	virtual void ExecuteAttack() override;
	virtual void BeginPlay() override;
};