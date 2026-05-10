#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RiftHUD.generated.h"

class UMainHUDWidget;
class ALoLChampion;

UCLASS()
class LEAGUEOFLEGENDS_API ARiftHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARiftHUD();

	virtual void BeginPlay() override;

	// PlayerController의 AcknowledgePossession에서 호출
	void InitHUD(ALoLChampion* Champion);
	void RefreshSkillIcons(ALoLChampion* Champion);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainHUDWidget> MainHUDClass;

private:
	UPROPERTY()
	TObjectPtr<UMainHUDWidget> MainHUDWidget;
};
