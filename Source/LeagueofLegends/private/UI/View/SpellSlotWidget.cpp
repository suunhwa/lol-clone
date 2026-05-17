#include "UI/View/SpellSlotWidget.h"

#include "LeagueofLegends.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void USpellSlotWidget::InitSlot(int32 InSlotIndex, ESummonerSpell InSpell)
{
	SlotIndex = InSlotIndex;
	Spell     = InSpell;

	if (Txt_Index)
	{
		Txt_Index->SetText(FText::FromString(SlotIndex == 0 ? TEXT("D") : TEXT("F")));
	}

	// 쿨타임 오버레이 초기 숨김
	if (Overlay_CD)
	{
		Overlay_CD->SetVisibility(ESlateVisibility::Hidden);
	}

	// 머티리얼 초기화
	if (Img_Cooldown)
	{
		CooldownMat = Img_Cooldown->GetDynamicMaterial();
		if (CooldownMat)
		{
			CooldownMat->SetScalarParameterValue(TEXT("CooldownRatio"), 1.f);
		}
	}

	RefreshIcon();
}

void USpellSlotWidget::TriggerCooldown(float Duration)
{
	LocalTotalCD   = Duration;
	LocalRemaining = Duration;
}

void USpellSlotWidget::RefreshIcon()
{
	if (!Img_Icon) { return; }

	if (TObjectPtr<UTexture2D>* Found = SpellIcons.Find(Spell))
	{
		Img_Icon->SetBrushFromTexture(Found->Get(), true);
		Img_Icon->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
	}
}

void USpellSlotWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	// 로컬 타이머 갱신
	if (LocalRemaining > 0.f)
	{
		LocalRemaining = FMath::Max(0.f, LocalRemaining - DeltaTime);
	}

	const bool  bOnCD     = LocalRemaining > 0.f;
	const float Remaining = LocalRemaining;

	// 머티리얼 지연 초기화 (렌더링 후 첫 틱에서 생성될 수 있음)
	if (!CooldownMat && Img_Cooldown)
	{
		CooldownMat = Img_Cooldown->GetDynamicMaterial();
	}

	if (Overlay_CD)
	{
		Overlay_CD->SetVisibility(bOnCD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (CooldownMat)
	{
		const float Ratio = (bOnCD && LocalTotalCD > 0.f) ? 1.f - (Remaining / LocalTotalCD) : 1.f;
		CooldownMat->SetScalarParameterValue(TEXT("CooldownRatio"), Ratio);
	}

	if (Txt_CoolTime)
	{
		if (bOnCD)
		{
			FString CDStr = (Remaining > 1.f)
				? FString::FromInt(FMath::CeilToInt(Remaining))
				: FString::Printf(TEXT("%.1f"), Remaining);
			Txt_CoolTime->SetText(FText::FromString(CDStr));
			Txt_CoolTime->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Txt_CoolTime->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
