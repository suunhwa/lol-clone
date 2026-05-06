#pragma once
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
	void Respawn();
	FTimerHandle RespawnTimerHandle;
};