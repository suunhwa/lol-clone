// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/PlayerHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"

void UPlayerHUDWidget::SetNickName(const FString& NickName)
{
	if (Txt_NickName)
	{
		Txt_NickName->SetText(FText::FromString(NickName));
	}
}

void UPlayerHUDWidget::SetLevel(int32 Level)
{
	if (Txt_Level)
	{
		Txt_Level->SetText(FText::AsNumber(Level));
	}
}

void UPlayerHUDWidget::SetHP(float Current, float Max)
{
	if (Progress_HP && Max > 0.f)
	{
		Progress_HP->SetPercent(Current / Max);
	}
}

void UPlayerHUDWidget::SetMP(float Current, float Max)
{
	if (Progress_MP && Max > 0.f)
	{
		Progress_MP->SetPercent(Current / Max);
	}
}

void UPlayerHUDWidget::SetMaxHP(float MaxHP)
{
	if (!Img_Graduation) return;

	// MID가 없으면 브러시 머티리얼로부터 생성
	if (!GraduationMID)
	{
		UMaterialInterface* BaseMat = Cast<UMaterialInterface>(Img_Graduation->GetBrush().GetResourceObject());
		if (!BaseMat) return;

		GraduationMID = UMaterialInstanceDynamic::Create(BaseMat, this);
		Img_Graduation->SetBrushFromMaterial(GraduationMID);
	}

	GraduationMID->SetScalarParameterValue(TEXT("MaxHealth"), MaxHP);
}

void UPlayerHUDWidget::SetHPBarType(EHPBarType Type)
{
	if (!Progress_HP) return;

	// FillImage 머티리얼에서 MID 생성
	if (!HPBarMID)
	{
		FProgressBarStyle Style = Progress_HP->GetWidgetStyle();
		UMaterialInterface* BaseMat = Cast<UMaterialInterface>(Style.FillImage.GetResourceObject());
		if (!BaseMat) return;

		HPBarMID = UMaterialInstanceDynamic::Create(BaseMat, this);

		// FillImage 브러시에 MID 적용
		Style.FillImage.SetResourceObject(HPBarMID);
		Progress_HP->SetWidgetStyle(Style);
	}

	FLinearColor MainColor;
	FLinearColor HighlightColor;

	switch (Type)
	{
	case EHPBarType::Self:
		MainColor      = FLinearColor(0.083435f, 1.0f,      0.0f,      1.0f);
		HighlightColor = FLinearColor(0.264838f, 1.0f,      0.197917f, 1.0f);
		break;
	case EHPBarType::Ally:
		MainColor      = FLinearColor(0.0f,      0.726066f, 0.921875f, 1.0f);
		HighlightColor = FLinearColor(0.206462f, 0.769919f, 0.921875f, 1.0f);
		break;
	case EHPBarType::Enemy:
	default:
		MainColor      = FLinearColor(1.0f,      0.064985f, 0.052083f, 1.0f);
		HighlightColor = FLinearColor(1.0f,      0.198559f, 0.187500f, 1.0f);
		break;
	}

	HPBarMID->SetVectorParameterValue(TEXT("MainColor"),      MainColor);
	HPBarMID->SetVectorParameterValue(TEXT("HighlightColor"), HighlightColor);
}

