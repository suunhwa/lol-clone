// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CooldownComponent.h"

UCooldownComponent::UCooldownComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCooldownComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCooldownComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

