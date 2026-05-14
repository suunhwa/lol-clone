#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PickWindowHUD.generated.h"

class UPickWindowWidget;
class UPickWindowViewModel;

UCLASS()
class LEAGUEOFLEGENDS_API APickWindowHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPickWindowWidget> PickWindowClass;

private:
	UPROPERTY()
	TObjectPtr<UPickWindowWidget> PickWindowWidget;
};
