#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ExitPopupWidget.generated.h"

class UButton;

UCLASS()
class LEAGUEOFLEGENDS_API UExitPopupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnClickExit();

	UFUNCTION()
	void OnClickClose();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Exit;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Close;
};
