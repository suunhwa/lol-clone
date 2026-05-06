#pragma once
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
};