#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RiftHUD.generated.h"

class UInventoryWidget;
class UShopViewModel;
class UShopWidget;
class UMainHUDWidget;
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
	
private:	
	void SetupMainHUD(ALoLChampion* Champion);
	void SetupShopMVVM(ALoLChampion* Champion);
	
protected:
	// Main HUD
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainHUDWidget> MainHUDClass;

	// Shop
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UShopWidget> ShopWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	// Shop
	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidget;

	UPROPERTY()
	TObjectPtr<UShopViewModel> ShopVM;
};

