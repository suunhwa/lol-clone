// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLMinion.h"

ALoLMinion::ALoLMinion()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLMinion::BeginPlay()
{
	Super::BeginPlay();
}

void ALoLMinion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

