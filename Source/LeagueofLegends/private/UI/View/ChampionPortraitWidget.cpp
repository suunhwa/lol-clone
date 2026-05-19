#include "UI/View/ChampionPortraitWidget.h"

#include "LeagueofLegends.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/ViewModel/ChampionPortraitViewModel.h"

void UChampionPortraitWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);

	if (Btn_Stats)
	{
		Btn_Stats->OnClicked.AddDynamic(this, &UChampionPortraitWidget::OnStatsClicked);
	}

	VM = Cast<UChampionPortraitViewModel>(InViewModel);
	if (!VM) { return; }

	VM->OnLevelUpdated.AddUObject(this, &UChampionPortraitWidget::OnLevelUpdated);

	// 초기값 즉시 반영
	if (Txt_Level)
	{
		Txt_Level->SetText(FText::AsNumber(FMath::Max(1, VM->GetLevel())));
	}
		

	if (Img_Portrait)
	{
		if (UTexture2D* Portrait = VM->GetPortraitTexture())
		{
			UMaterialInstanceDynamic* DynMat = Img_Portrait->GetDynamicMaterial();
			if (DynMat)
			{
				DynMat->SetTextureParameterValue(TEXT("PortraitTex"), Portrait);
			}
		}
	}
	
	// 사망 오버레이 초기 숨김
	if (Img_DeadOverlay)
	{
		Img_DeadOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Txt_DeathTimer)
	{
		Txt_DeathTimer->SetVisibility(ESlateVisibility::Hidden);
	}

	// XP 머티리얼 Dynamic Instance 캐싱
	if (Img_Exp)
	{
		ExpMat = Img_Exp->GetDynamicMaterial();
		if (ExpMat)
		{
			ExpMat->SetScalarParameterValue(TEXT("Progress"), 0.f);
		}
	}
}

void UChampionPortraitWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(VM)) { return; }

	// 사망 오버레이 + 타이머 갱신
	const bool bDead = VM->IsChampionDead();
	const ESlateVisibility DeadVis = bDead ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;

	if (Img_DeadOverlay)
	{
		Img_DeadOverlay->SetVisibility(DeadVis);
	}

	if (Txt_DeathTimer)
	{
		Txt_DeathTimer->SetVisibility(DeadVis);
		if (bDead)
		{
			const int32 SecsLeft = FMath::CeilToInt(VM->GetRespawnTimeRemaining());
			Txt_DeathTimer->SetText(FText::AsNumber(FMath::Max(0, SecsLeft)));
		}
	}

	// XP 바 갱신 (기존 로직 유지)
	TickAccum += InDeltaTime;
	if (TickAccum < RefreshInterval) { return; }
	TickAccum = 0.f;

	if (ExpMat)
	{
		ExpMat->SetScalarParameterValue(TEXT("Progress"), VM->GetXPProgress());
	}
}

void UChampionPortraitWidget::OnStatsClicked()
{
	OnStatsToggleRequested.Broadcast();
}

void UChampionPortraitWidget::OnLevelUpdated(int32 NewLevel)
{
	PRINTLOG_SH(TEXT("[Portrait] Level updated → %d"), NewLevel);
	if (Txt_Level)
	{
		Txt_Level->SetText(FText::AsNumber(NewLevel));
	}
}
