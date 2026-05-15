#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h" // 부모 클래스 헤더
#include "LoLMinion_Melee.generated.h"

/**
 * 전사 미니언 (ID: 3001)
 */


UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Melee : public ALoLMinion
{
	GENERATED_BODY()

public:
	// 생성자에서 ID 및 기본 에셋 설정
	ALoLMinion_Melee();
	virtual void ExecuteAttack() override;
protected:
	// 전사 미니언만의 특별한 시작 로직이 필요할 경우 사용
	virtual void BeginPlay() override;
	
};
