// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModelBase.h"
#include "InventoryViewModel.generated.h"

class UInventoryComponent;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UInventoryViewModel : public UViewModelBase
{
	GENERATED_BODY()
	
public:
	virtual void Initialize() override;
	virtual void Reset() override;
	
	void SetInventoryComponent(UInventoryComponent* InInventoryComp) { InventoryComp = InInventoryComp; }
	
	
private:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComp;
};
