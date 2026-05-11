#include "UI/View/SkillSlotWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/CooldownComponent.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/RiftPlayerController.h"

void USkillSlotWidget::InitSlot(UCooldownComponent* CD, FName InCDTag, FText HotkeyText, float InTotalCD,
                                float InManaCost)
{
	CooldownComp = CD;
	CDTag = InCDTag;
	TotalCD = InTotalCD;

	if (Txt_Index)
	{
		Txt_Index->SetText(HotkeyText);
	}

	if (Txt_ManaCost)
	{
		Txt_ManaCost->SetText(FText::FromString(FString::FromInt(FMath::RoundToInt(InManaCost))));
	}

	if (Overlay_CD)
	{
		Overlay_CD->SetVisibility(ESlateVisibility::Hidden);
	}

	if (Img_Cooldown)
	{
		CooldownMat = Img_Cooldown->GetDynamicMaterial();
	}

	if (Btn_LevelUp)
	{
		Btn_LevelUp->OnClicked.AddDynamic(this, &USkillSlotWidget::OnLevelUpClicked);
		Btn_LevelUp->SetVisibility(ESlateVisibility::Hidden);
	}

	RefreshRank(0);
}

void USkillSlotWidget::InitSlotMeta(ESkillSlot InSlot, int32 InMaxRank)
{
	SkillSlot = InSlot;
	MaxRank = InMaxRank;

	// MaxRank=0이면 패시브 — 높이 유지하면서 숨김
	if (MaxRank == 0)
	{
		if (Btn_LevelUp) Btn_LevelUp->SetVisibility(ESlateVisibility::Hidden);
		if (HBox_Ranks) HBox_Ranks->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	// Q/W/E/R — 버튼/점 컨테이너 Visible 복구
	if (HBox_Ranks)
	{
		HBox_Ranks->SetVisibility(ESlateVisibility::Visible);
	}

	if (Btn_LevelUp)
	{
		Btn_LevelUp->SetVisibility(ESlateVisibility::Hidden);
	}

	for (int32 i = 1; i <= 5; i++)
	{
		if (UImage* Dot = GetDot(i))
		{
			Dot->SetVisibility(i <= MaxRank ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
	}
}

void USkillSlotWidget::SetIcon(UTexture2D* Texture)
{
	if (Img_Icon && Texture)
	{
		Img_Icon->SetBrushFromTexture(Texture, true);
	}
}

void USkillSlotWidget::RefreshRank(int32 NewRank)
{
	CurrentRank = NewRank;

	for (int32 i = 1; i <= MaxRank; i++)
	{
		UImage* Dot = GetDot(i);
		if (!Dot) { continue; }

		const bool bFilled = i <= CurrentRank;
		if (bFilled && DotFilledTexture)
		{
			Dot->SetBrushFromTexture(DotFilledTexture, true);
		}

		else if (!bFilled && DotEmptyTexture)
		{
			Dot->SetBrushFromTexture(DotEmptyTexture, true);
		}
	}
}

void USkillSlotWidget::SetSkillActive(bool bActive)
{
	if (Img_Icon)
	{
		Img_Icon->SetColorAndOpacity(bActive
			                             ? FLinearColor(1.f, 1.f, 1.f, 1.f)
			                             : FLinearColor(0.25f, 0.25f, 0.25f, 1.f));
	}
}

void USkillSlotWidget::SetLevelUpAvailable(bool bAvailable)
{
	if (Btn_LevelUp)
	{
		Btn_LevelUp->SetVisibility(bAvailable ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
		
}

void USkillSlotWidget::OnLevelUpClicked()
{
	if (ARiftPlayerController* PC = Cast<ARiftPlayerController>(GetOwningPlayer()))
	{
		PC->Server_AssignSkillPoint(SkillSlot);
	}
}

UImage* USkillSlotWidget::GetDot(int32 Index) const
{
	switch (Index)
	{
	case 1: return Img_Dot_1;
	case 2: return Img_Dot_2;
	case 3: return Img_Dot_3;
	case 4: return Img_Dot_4;
	case 5: return Img_Dot_5;
	default: return nullptr;
	}
}

void USkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CooldownComp || CDTag.IsNone()) { return; }

	const bool bOnCD = CooldownComp->IsOnCooldown(CDTag);
	const float Remaining = CooldownComp->GetRemaining(CDTag);

	if (Overlay_CD)
	{
		Overlay_CD->SetVisibility(bOnCD ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (CooldownMat)
	{
		const float Ratio = (bOnCD && TotalCD > 0.f) ? 1.f - (Remaining / TotalCD) : 1.f;
		CooldownMat->SetScalarParameterValue(TEXT("CooldownRatio"), Ratio);
	}

	if (Txt_CoolTime && bOnCD)
	{
		FString CDStr = (Remaining > 1.f)
			                ? FString::FromInt(FMath::CeilToInt(Remaining))
			                : FString::Printf(TEXT("%.1f"), Remaining);
		Txt_CoolTime->SetText(FText::FromString(CDStr));
	}
}
