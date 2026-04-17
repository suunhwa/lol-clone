// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLCharacterBase.h"

ALoLCharacterBase::ALoLCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ALoLCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALoLCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

