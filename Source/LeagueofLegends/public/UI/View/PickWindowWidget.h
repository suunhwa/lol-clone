#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "PickWindowWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UWrapBox;
class UTeamSlotWidget;
class UEnemySlotWidget;
class UChampSlotWidget;
class UPickWindowViewModel;
struct FChampSlotViewData;

UCLASS()
class LEAGUEOFLEGENDS_API UPickWindowWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;
	virtual void UnbindViewModel() override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_moveBlueTeam;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_moveRedTeam;

	// 호스트: START GAME / 클라이언트: READY
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_Ready;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> txt_ready_label;

	// 팀 슬롯 컨테이너
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_MyTeam;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_EnemyTeam;

	// 챔피언 슬롯 WrapBox
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> WrapBox_Champs;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UChampSlotWidget> ChampSlotClass;

	UFUNCTION() 
	void OnMoveBlueTeam();
	
	UFUNCTION() 
	void OnMoveRedTeam();
	
	UFUNCTION() 
	void OnReadyOrStart();

	void OnPickWindowUpdated();
	void OnChampionListReady(const TArray<FChampSlotViewData>& ChampList);
	void UpdateReadyButton();
	void RefreshTeamSlots();

	float RefreshAccum = 0.f;
	static constexpr float RefreshInterval = 0.5f;
};
