// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/SlotWidgetBase.h"
#include "Components/SkillComponent.h"
#include "SkillSlotWidget.generated.h"

class USizeBox;
class UTextBlock;
class UOverlay;
class UButton;
class UHorizontalBox;
class UImage;
class UCooldownComponent;
class UMaterialInstanceDynamic;

UCLASS()
class LEAGUEOFLEGENDS_API USkillSlotWidget : public USlotWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// 슬롯 초기화: 쿨다운 컴포넌트, 태그("Skill.Q" 등), 단축키 텍스트("Q"), 최대 쿨다운, 마나 코스트
	void InitSlot(UCooldownComponent* CD, FName InCDTag, FText HotkeyText, float InTotalCD, float InManaCost);
	void InitSlotMeta(ESkillSlot InSlot, int32 InMaxRank);
	void SetIcon(UTexture2D* Texture);
	void RefreshRank(int32 NewRank);
	void SetLevelUpAvailable(bool bAvailable);
	void SetSkillActive(bool bActive); // 랭크 0이면 어둡게
	
	/*UFUNCTION(BlueprintCallable)
	void SetSlotSize(float Width, float Height);
	
	UFUNCTION(BlueprintCallable)
	void SetPassive();*/

protected:
	/*UPROPERTY(meta = (BindWidget))
	USizeBox* Root_SizeBox;*/
		
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_CD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ManaCost;

	// 레벨업 버튼
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_LevelUp;

	// skill 레벨업 컨테이너 (패시브: Collapsed)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBox_Ranks;

	// skill 레벨업 (최대 5개, R은 3개까지만 사용)
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Img_Dot_1;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Img_Dot_2;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Img_Dot_3;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Img_Dot_4;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Img_Dot_5;

	// WBP에서 채워진/빈 점 텍스처 지정
	UPROPERTY(EditDefaultsOnly, Category = "Rank")
	TObjectPtr<UTexture2D> DotFilledTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Rank")
	TObjectPtr<UTexture2D> DotEmptyTexture;

private:
	UFUNCTION()
	void OnLevelUpClicked();

	UImage* GetDot(int32 Index) const;

	UPROPERTY()
	TObjectPtr<UCooldownComponent> CooldownComp;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMat;

	FName CDTag;
	float TotalCD = 0.f;
	ESkillSlot SkillSlot = ESkillSlot::Q;
	int32 MaxRank = 5;
	int32 CurrentRank = 0;
};

