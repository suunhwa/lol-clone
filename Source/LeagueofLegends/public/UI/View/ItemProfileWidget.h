// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "ItemProfileWidget.generated.h"

class UCheckBox;
class UImage;
class UTextBlock;
/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnItemProfileClicked, int32, UItemProfileWidget*);
UCLASS()
class LEAGUEOFLEGENDS_API UItemProfileWidget : public UWidgetViewBase
{
	GENERATED_BODY()
	
public:
	void SetItemProfile(UTexture2D* Icon, const FString& Price, int32 InItemID);
	void SetItemIcon(UTexture2D* Icon);
	void SetItemPrice(const FString& Price);

	FOnItemProfileClicked OnItemClicked;

	void BindItemButtonClick();
	
	void SetSelected(bool bSelected);
	
private:
	UFUNCTION()
	void HandleToggleStateChanged(bool bIsChecked);
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Tgl_ItemProfile;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_OutlineL;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_OutlineT;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_OutlineR;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_OutlineB;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Price;
	
	UPROPERTY(VisibleAnywhere, Category="ItemProfile")
	int32 ItemID;
};
