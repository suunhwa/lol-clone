// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/View/WidgetViewBase.h"

void UWidgetViewBase::BindViewModel(UViewModelBase* InViewModel)
{
	OwnerViewModel = InViewModel;
	ensureMsgf(false, TEXT("BindViewModel must be overridden in %s"), *GetClass()->GetName());
}

