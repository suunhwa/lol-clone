// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Type/RiftTypes.h"
#include "LoLPlayerStart.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALoLPlayerStart(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	ETeam Team = ETeam::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 SlotIndex = 0;
};
