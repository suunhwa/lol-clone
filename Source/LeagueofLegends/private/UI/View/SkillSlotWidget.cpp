#include "UI/View/SkillSlotWidget.h"

#include "Components/CooldownComponent.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void USkillSlotWidget::InitSlot(UCooldownComponent* CD, FName InCDTag, FText HotkeyText, float InTotalCD, float InManaCost)
{
	CooldownComp = CD;
	CDTag = InCDTag;
	TotalCD = InTotalCD;

	if (Txt_Index)
		Txt_Index->SetText(HotkeyText);

	if (Txt_ManaCost)
		Txt_ManaCost->SetText(FText::FromString(FString::FromInt(FMath::RoundToInt(InManaCost))));

	if (Overlay_CD)
		Overlay_CD->SetVisibility(ESlateVisibility::Hidden);

	// M_Cooldown Dynamic Instance 캐싱
	if (Img_Cooldown)
		CooldownMat = Img_Cooldown->GetDynamicMaterial();
}

void USkillSlotWidget::SetIcon(UTexture2D* Texture)
{
	if (Img_Icon && Texture)
		Img_Icon->SetBrushFromTexture(Texture, true);
}

void USkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CooldownComp || CDTag.IsNone()) return;

	const bool bOnCD = CooldownComp->IsOnCooldown(CDTag);
	const float Remaining = CooldownComp->GetRemaining(CDTag);

	// Overlay_CD 일괄 토글
	if (Overlay_CD)
		Overlay_CD->SetVisibility(bOnCD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

	// CooldownRatio: 0 = 꽉참(쿨타임 시작), 1 = 비어있음(쿨타임 종료)
	if (CooldownMat)
	{
		const float Ratio = (bOnCD && TotalCD > 0.f) ? 1.f - (Remaining / TotalCD) : 1.f;
		CooldownMat->SetScalarParameterValue(TEXT("CooldownRatio"), Ratio);
	}

	// 쿨타임
	if (Txt_CoolTime && bOnCD)
	{
		FString CDStr = (Remaining > 1.f)
			? FString::FromInt(FMath::CeilToInt(Remaining))
			: FString::Printf(TEXT("%.1f"), Remaining);
		Txt_CoolTime->SetText(FText::FromString(CDStr));
	}
}
