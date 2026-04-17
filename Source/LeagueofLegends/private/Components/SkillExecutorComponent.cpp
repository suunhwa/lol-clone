// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/SkillExecutorComponent.h"

USkillExecutorComponent::USkillExecutorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USkillExecutorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USkillExecutorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

