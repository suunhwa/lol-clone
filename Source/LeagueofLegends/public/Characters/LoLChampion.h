// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "LoLChampion.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLChampion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLChampion();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	
};

