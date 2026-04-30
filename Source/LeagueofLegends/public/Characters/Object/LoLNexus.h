#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLStructure.h"
#include "LolNexus.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALolNexus : public ALoLStructure
{
	GENERATED_BODY()

public:
	ALolNexus();

protected:
	virtual void OnDestroyed() override;
};