#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "PlayerStatsViewModel.generated.h"

class ALoLChampion;
class UItemInstance;

DECLARE_MULTICAST_DELEGATE(FOnStatsRefresh);

UCLASS()
class LEAGUEOFLEGENDS_API UPlayerStatsViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override {}

	void Setup(ALoLChampion* InChampion);

	FOnStatsRefresh OnStatsRefresh;

	float GetAD() const;
	float GetAP() const;
	float GetArmor() const;
	float GetMR() const;
	float GetAS() const;
	float GetAH() const;
	float GetCrit() const;
	float GetMS() const;

private:
	UFUNCTION()
	void HandleLevelChanged(int32 NewLevel);

	UFUNCTION()
	void HandleInventoryChanged(int32 SlotIndex, UItemInstance* Item);

	UPROPERTY()
	TObjectPtr<ALoLChampion> Champion;
};
