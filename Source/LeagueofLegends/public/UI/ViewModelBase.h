// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ViewModelBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class LEAGUEOFLEGENDS_API UViewModelBase : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialize() PURE_VIRTUAL(UViewModelBase::Initialize,);
	virtual void Reset() {}
};
