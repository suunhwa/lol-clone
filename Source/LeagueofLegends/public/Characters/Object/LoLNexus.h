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
	// 부모의 OnDestroyed를 무시하고 넥서스만의 연출용으로 재정의합니다.
	virtual void OnDestroyed() override;

	// 최종 폭발 및 슬로우 모션 처리
	void HandleGameOver();
	
private:
	FTimerHandle GameOverTimerHandle;	

};