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

	// 재생성 로직 (나중에 구현)
	void Respawn();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Inhibitor")
	float RespawnTime;

	FTimerHandle RespawnTimerHandle;
};