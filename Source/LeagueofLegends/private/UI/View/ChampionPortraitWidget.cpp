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
	if (txt_Level)
	{
		txt_Level->SetText(FText::AsNumber(FMath::Max(1, VM->GetLevel())));
	}
		

	if (img_Portrait)
	{
		if (UTexture2D* Portrait = VM->GetPortraitTexture())
		{
			UMaterialInstanceDynamic* DynMat = img_Portrait->GetDynamicMaterial();
			if (DynMat)
			{
				DynMat->SetTextureParameterValue(TEXT("PortraitTex"), Portrait);
			}
		}
	}

	// XP 머티리얼 Dynamic Instance 캐싱
	if (img_Exp)
	{
		ExpMat = img_Exp->GetDynamicMaterial();
		if (ExpMat)
		{
			ExpMat->SetScalarParameterValue(TEXT("Progress"), 0.f);
		}
	}
}

void UChampionPortraitWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(VM) || !ExpMat) { return; }

	TickAccum += InDeltaTime;
	if (TickAccum < RefreshInterval) { return; }
	TickAccum = 0.f;

	ExpMat->SetScalarParameterValue(TEXT("Progress"), VM->GetXPProgress());
}

void UChampionPortraitWidget::OnStatsClicked()
{
	OnStatsToggleRequested.Broadcast();
}

void UChampionPortraitWidget::OnLevelUpdated(int32 NewLevel)
{
	PRINTLOG_SH(TEXT("[Portrait] Level updated → %d"), NewLevel);
	if (txt_Level)
	{
		txt_Level->SetText(FText::AsNumber(NewLevel));
	}
}
