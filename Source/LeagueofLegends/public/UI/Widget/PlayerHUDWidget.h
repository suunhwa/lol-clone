// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;

UENUM(BlueprintType)
enum class EHPBarType : uint8
{
	Self,
	Ally,
	Enemy
};

/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetNickName(const FString& NickName);
	void SetLevel(int32 Level);
	void SetHP(float Current, float Max);
	void SetMP(float Current, float Max);
	void SetMaxHP(float MaxHP);
	void SetHPBarType(EHPBarType Type);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_NickName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Level;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Progress_HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Progress_MP;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Graduation;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GraduationMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HPBarMID;
};
