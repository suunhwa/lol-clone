// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WidgetViewBase.generated.h"

class UViewModelBase;

/**
 * 
 */
UCLASS(Abstract)
class LEAGUEOFLEGENDS_API UWidgetViewBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// HUD에서 ViewModel 주입 후 호출
	virtual void BindViewModel(UViewModelBase* InViewModel);
	virtual void UnbindViewModel() {}

protected:
	UPROPERTY()
	TObjectPtr<UViewModelBase> OwnerViewModel;
};

