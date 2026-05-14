#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RiftHUD.generated.h"

class UInventoryWidget;
class UShopViewModel;
class UShopWidget;
class UMainHUDWidget;
class UPickWindowWidget;
class ALoLChampion;

UCLASS()
class LEAGUEOFLEGENDS_API ARiftHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARiftHUD();

protected:
	virtual void BeginPlay() override;

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

	// 챔피언 선택창 (Lv_Lobby에서 자동 표시)
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPickWindowWidget> PickWindowClass;

private:
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;

	UPROPERTY()
	TObjectPtr<UPickWindowWidget> PickWindowWidget;

	// Shop
	UPROPERTY()
	TObjectPtr<UShopWidget> ShopWidget;

	UPROPERTY()
	TObjectPtr<UShopViewModel> ShopVM;
};

