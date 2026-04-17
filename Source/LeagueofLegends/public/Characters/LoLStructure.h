// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LoLStructure.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLStructure : public AActor
{
	GENERATED_BODY()

public:
	ALoLStructure();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};

