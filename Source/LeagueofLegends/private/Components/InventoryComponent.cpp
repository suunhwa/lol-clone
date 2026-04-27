// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInventoryComponent::PurchaseItem(UItemDataAsset* ItemData)
{
}

void UInventoryComponent::SellItem(int32 SlotIndex)
{
}

void UInventoryComponent::UndoPurchase()
{
}

bool UInventoryComponent::EquipTrinket(UItemDataAsset* TrinketData)
{
}

