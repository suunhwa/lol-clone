// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

ALoLChampion::ALoLChampion()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLChampion::BeginPlay()
{
	Super::BeginPlay();
}

void ALoLChampion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

