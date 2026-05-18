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
class USoundBase;
class UAudioComponent;

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
	
	// 서버: 세션 종료 후 Lobby로 돌아감, 클라이언트: 세션에서 나가고 Lobby로 돌아감.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_quit;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> txt_ready_label;

	// 팀 슬롯 컨테이너
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_BlueTeam;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_RedTeam;

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
	
	UFUNCTION()
	void OnQuit();

	void OnPickWindowUpdated();
	void OnChampionListReady(const TArray<FChampSlotViewData>& ChampList);
	void UpdateReadyButton();
	void RefreshTeamSlots();

	float RefreshAccum = 0.f;
	static constexpr float RefreshInterval = 0.5f;
	
private:
	// --- 픽창 BGM 설정을 위한 변수 추가 ---
	UPROPERTY(EditDefaultsOnly, Category = "UI|Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> PickWindowBGM;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;

	void PlayLobbyMusic();
	void StopLobbyMusic(float FadeOutTime = 1.0f);
	
	// ------------------------------------
};
