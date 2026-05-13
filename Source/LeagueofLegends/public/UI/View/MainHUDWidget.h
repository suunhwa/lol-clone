#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

class UMinimapWidget;
class UInventoryWidget;
class USkillBarWidget;
class UChampionPortraitWidget;
class UGameInfoBarWidget;
class UPlayerStatsWidget;
class ALoLChampion;
class ARiftPlayerState;
class ARiftGameState;

// WBP_MainHUD의 C++ 부모 클래스
// 하위 인스턴스 이름: SkillBar / ChampionPortrait / GameInfoBar
UCLASS()
class LEAGUEOFLEGENDS_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitHUD(ALoLChampion* Champion, ARiftPlayerState* PS, ARiftGameState* GS);
	USkillBarWidget* GetSkillBar() const { return SkillBar; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USkillBarWidget> SkillBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UChampionPortraitWidget> ChampionPortrait;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGameInfoBarWidget> GameInfoBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryWidget> WBP_Inventory;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMinimapWidget> WBP_Minimap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPlayerStatsWidget> PlayerStats;

private:
	UFUNCTION()
	void OnStatsToggleRequested();
};
