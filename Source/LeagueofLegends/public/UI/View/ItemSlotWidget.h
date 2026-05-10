// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/View/SlotWidgetBase.h"
#include "ItemSlotWidget.generated.h"

/**
 * 아이템 슬롯 위젯.
 * 공통 슬롯 요소는 USlotWidgetBase에서 상속받습니다.
 * 아이템 슬롯 고유 위젯 컴포넌트가 생기면 이 클래스에 추가하세요.
 */
UCLASS()
class LEAGUEOFLEGENDS_API UItemSlotWidget : public USlotWidgetBase
{
	GENERATED_BODY()
	
private:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Stack;
};

