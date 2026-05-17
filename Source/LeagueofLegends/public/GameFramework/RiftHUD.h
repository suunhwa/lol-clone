#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RiftHUD.generated.h"

class UInventoryWidget;
class UShopViewModel;
class UShopWidget;
class UMainHUDWidget;
class UExitPopupWidget;
class USkillBarWidget;
class ALoLChampion;

UCLASS()
class LEAGUEOFLEGENDS_API ARiftHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARiftHUD();

public:
	// PlayerController의 AcknowledgePossession에서 호출
	void InitHUD(ALoLChampion* Champion);
	void RefreshSkillIcons(ALoLChampion* Champion);

	void ToggleShop();
	void ToggleExitPopup();
	void InitSpellSlots(ALoLChampion* Champion);
	UMainHUDWidget* GetMainHUDWidget() const { return MainHUDWidget; }

private:
	void SetupMainHUD(ALoLChampion* Champion);
	void SetupShopMVVM(ALoLChampion* Champion);
	void SetupExitPopup();
	
protected:
	// Main HUD
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainHUDWidget> MainHUDClass;

	// Shop
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShopWidget> ShopWidgetClass;

	// Exit Popup
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UExitPopupWidget> ExitPopupClass;


private:
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	// Shop
	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidget;

	UPROPERTY()
	TObjectPtr<UShopViewModel> ShopVM;

	// Exit Popup
	UPROPERTY()
	TObjectPtr<UExitPopupWidget> ExitPopupWidget;
};

