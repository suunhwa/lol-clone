// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLStructure.h"

ALoLStructure::ALoLStructure()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLStructure::BeginPlay()
{
	Super::BeginPlay();
}

void ALoLStructure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

