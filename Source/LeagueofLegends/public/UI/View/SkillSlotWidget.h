// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/SlotWidgetBase.h"
#include "SkillSlotWidget.generated.h"

class UTextBlock;
class UOverlay;
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
	void SetIcon(UTexture2D* Texture);

protected:
	// 쿨타임 관련 위젯 컨테이너 — Hidden/Visible 일괄 처리
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_CD;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ManaCost;

private:
	UPROPERTY()
	TObjectPtr<UCooldownComponent> CooldownComp;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CooldownMat;

	FName CDTag;
	float TotalCD = 0.f;
};

