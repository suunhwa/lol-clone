#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "ChampionPortraitWidget.generated.h"

class UTextBlock;
class UImage;
class UChampionPortraitViewModel;

// WBP_ChampionPortrait의 C++ 부모 클래스
UCLASS()
class LEAGUEOFLEGENDS_API UChampionPortraitWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void OnLevelUpdated(int32 NewLevel);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> txt_Level;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> img_Portrait;

	// M_Exp Progress 파라미터 제어
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> img_Exp;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ExpMat;

	UPROPERTY()
	TObjectPtr<UChampionPortraitViewModel> VM;

	float TickAccum = 0.f;
	static constexpr float RefreshInterval = 0.1f;
};
