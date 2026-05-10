// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "SlotWidgetBase.generated.h"

class UImage;
class UTextBlock;

/**
 * SkillSlot / ItemSlot 공통 슬롯 위젯 베이스.
 * WBP에서 동일하게 존재하는 위젯 컴포넌트를 바인딩합니다.
 */
UCLASS(Abstract)
class LEAGUEOFLEGENDS_API USlotWidgetBase : public UWidgetViewBase
{
	GENERATED_BODY()
	
protected:
	// ─── 공통 이미지 ───────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_GoldOutline;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Cooldown;

	// ─── 공통 텍스트 ───────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_CoolTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Index;
};

