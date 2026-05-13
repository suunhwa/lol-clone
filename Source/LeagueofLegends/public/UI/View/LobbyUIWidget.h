#pragma once

#include "CoreMinimal.h"
#include "UI/View/WidgetViewBase.h"
#include "LobbyUIWidget.generated.h"

class UButton;
class ULobbyUIViewModel;
class USessionWidget;
class USessionViewModel;

UCLASS()
class LEAGUEOFLEGENDS_API ULobbyUIWidget : public UWidgetViewBase
{
	GENERATED_BODY()

public:
	virtual void BindViewModel(UViewModelBase* InViewModel) override;
	virtual void UnbindViewModel() override;

	// LobbyHUD에서 호출 — 내부 WBP_Session에 ViewModel 전달
	void BindSessionWidget(USessionViewModel* InVM);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_exitgame;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USessionWidget> WBP_Session;

	UFUNCTION() 
	void OnLolClicked();
	
	UFUNCTION() 
	void OnExitClicked();
};
