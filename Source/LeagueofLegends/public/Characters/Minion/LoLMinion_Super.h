#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Super.generated.h"

/**
 * 슈퍼 미니언 (ID: 3004)
 * 근접 공격 유닛이며, LoLMinion의 기본 AI와 A* 이동 로직을 그대로 사용합니다.
 */
UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Super : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Super();

protected:
	virtual void BeginPlay() override;

	// 슈퍼 미니언 전용 로직이 필요할 경우 오버라이드 (예: 주변 미니언 버프 등)
	// virtual void Tick(float DeltaTime) override;
};