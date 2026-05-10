#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "GameInfoBarWidget.generated.h"

class UTextBlock;
class UGameInfoViewModel;

// WBP_GameInfoBar의 C++ 부모 클래스
UCLASS()
class LEAGUEOFLEGENDS_API UGameInfoBarWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_BlueKills;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_RedKills;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_KDA;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_CS;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_GameTime;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_FPS;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Ping;

private:
	void Refresh();

	UPROPERTY()
	TObjectPtr<UGameInfoViewModel> VM;

	float TickAccum = 0.f;
	static constexpr float RefreshInterval = 0.5f;
};
