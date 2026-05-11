// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "ItemProfileWidget.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemProfileClicked, int32);
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
	
private:
	UFUNCTION()
	void HandleClicked();
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_ItemProfile;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Price;
	
	UPROPERTY(VisibleAnywhere, Category="ItemProfile")
	int32 ItemID;
};

