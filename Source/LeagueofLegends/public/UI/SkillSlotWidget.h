// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlotWidgetBase.h"
#include "SkillSlotWidget.generated.h"

class UTextBlock;

/**
 * 스킬 슬롯 위젯.
 * 공통 슬롯 요소는 UCommonSlotWidget에서 상속받으며,
 * 스킬 슬롯 고유 위젯 컴포넌트를 추가로 바인딩합니다.
 */
UCLASS()
class LEAGUEOFLEGENDS_API USkillSlotWidget : public USlotWidgetBase
{
	GENERATED_BODY()

protected:
	// ─── SkillSlot 고유 텍스트 ─────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ManaCost;
};
