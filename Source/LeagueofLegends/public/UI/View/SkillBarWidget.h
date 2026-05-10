#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "SkillBarWidget.generated.h"

class USkillSlotWidget;
class UImage;
class UTextBlock;
class UChampionData;
class USkillBarViewModel;

// WBP_SkillBar의 C++ 부모 클래스
UCLASS()
class LEAGUEOFLEGENDS_API USkillBarWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;

	void RefreshIcons(UChampionData* Data);

private:
	UFUNCTION()
	void OnHPChanged(float Current, float Max);

	UFUNCTION()
	void OnManaChanged(float Current, float Max);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> Slot_P;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> Slot_Q;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> Slot_W;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> Slot_E;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillSlotWidget> Slot_R;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_HP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_MP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_HP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_MP;

private:
	UPROPERTY()
	TObjectPtr<USkillBarViewModel> VM;
};
