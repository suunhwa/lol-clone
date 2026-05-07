// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "FOWVolume.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API AFOWVolume : public AVolume
{
	GENERATED_BODY()

public:
	AFOWVolume();
	
	FBox GetFOWBounds() const
	{
		FVector Origin;
		FVector BoxExtent;
		GetActorBounds(false, Origin, BoxExtent);
		return FBox(Origin - BoxExtent, Origin + BoxExtent);
	}
};
