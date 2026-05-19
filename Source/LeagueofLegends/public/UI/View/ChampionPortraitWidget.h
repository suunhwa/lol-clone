#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "ChampionPortraitWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UChampionPortraitViewModel;

DECLARE_MULTICAST_DELEGATE(FOnStatsToggleRequested);

// WBP_ChampionPortrait의 C++ 부모 클래스
UCLASS()
class LEAGUEOFLEGENDS_API UChampionPortraitWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;

	FOnStatsToggleRequested OnStatsToggleRequested;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void OnLevelUpdated(int32 NewLevel);

	UFUNCTION()
	void OnStatsClicked();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Level;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Portrait;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_DeadOverlay;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_DeathTimer;

	// M_Exp Progress 파라미터 제어
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Exp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Stats;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ExpMat;

	UPROPERTY()
	TObjectPtr<UChampionPortraitViewModel> VM;

	float TickAccum = 0.f;
	static constexpr float RefreshInterval = 0.1f;
};
