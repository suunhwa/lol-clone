// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/DescStatWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDescStatWidget::SetDescStatWidget(const FText& Name, UTexture2D* Icon, const FText& Value)
{
	SetStatName(Name);
	SetStatIcon(Icon);
	SetStatValue(Value);
}

void UDescStatWidget::SetStatName(const FText& Name)
{
	if (Txt_StatName)
	{
		Txt_StatName->SetText(Name);
	}
}

void UDescStatWidget::SetStatIcon(UTexture2D* Icon)
{
	if (Img_StatIcon && Icon)
	{
		Img_StatIcon->SetBrushFromTexture(Icon);
	}
}

void UDescStatWidget::SetStatValue(const FText& Value)
{
	if (Txt_StatValue)
	{
		Txt_StatValue->SetText(Value);
	}
}
