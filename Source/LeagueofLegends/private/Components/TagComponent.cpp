// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/TagComponent.h"

UTagComponent::UTagComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTagComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTagComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

