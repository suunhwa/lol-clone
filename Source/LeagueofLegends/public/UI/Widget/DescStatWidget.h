// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DescStatWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UDescStatWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetDescStatWidget(const FText& Name, UTexture2D* Icon, const FText& Value);
	void SetStatName(const FText& Name);
	void SetStatIcon(UTexture2D* Icon);
	void SetStatValue(const FText& Value);
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_StatIcon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatValue;
};
