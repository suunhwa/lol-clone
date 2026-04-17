// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LoLCharacterBase.generated.h"

UCLASS(Abstract)
class LEAGUEOFLEGENDS_API ALoLCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ALoLCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

