// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/RiftTypes.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LEAGUEOFLEGENDS_API IDamageable
{
	GENERATED_BODY()

public:
	virtual void ReceiveDamage(float Amount, EDamageType DamageType, AActor* DamageInstigator) = 0;
	virtual bool IsDead() const = 0;
};
