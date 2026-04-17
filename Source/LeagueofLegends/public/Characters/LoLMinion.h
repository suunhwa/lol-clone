// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "LoLMinion.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLMinion();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};

