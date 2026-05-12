// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetViewBase.h"
#include "MinimapWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UMinimapWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetMinimapFOWTexture(UTexture2D* NewTexture);
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_MinimapFOW;
};
