#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLStructure.h"
#include "LoLInhibitor.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLInhibitor : public ALoLStructure
{
	GENERATED_BODY()

public:
	ALoLInhibitor();

protected:
	virtual void OnDestroyed() override;

	// 부활 로직
	void Respawn();

	FTimerHandle RespawnTimerHandle;

	// 스포너 제어 (True면 슈퍼 미니언, False면 일반)
	void UpdateSpawner(bool bSpawnSuper);
};