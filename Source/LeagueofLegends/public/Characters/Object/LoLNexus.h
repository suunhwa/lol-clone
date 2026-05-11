#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLStructure.h"
#include "LoLNexus.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLNexus : public ALoLStructure
{
	GENERATED_BODY()

public:
	ALoLNexus();

protected:
	virtual void OnDestroyed() override;

	// 알파 발표용 간단한 종료 연출 함수
	void HandleGameOver();
};