#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/ViewModelBase.h"
#include "SkillBarViewModel.generated.h"

class ALoLChampion;
class UCooldownComponent;
class UChampionData;
class UStatComponent;
class USkillComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStatValueChanged, float /*Current*/, float /*Max*/);

UCLASS()
class LEAGUEOFLEGENDS_API USkillBarViewModel : public UViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override {}

	void Setup(ALoLChampion* InChampion);

	UCooldownComponent* GetCooldownComp() const;
	UChampionData* GetChampionData() const;
	UStatComponent*  GetStatComp()  const;
	USkillComponent* GetSkillComp() const;

	FOnStatValueChanged OnHPChanged;
	FOnStatValueChanged OnManaChanged;

private:
	UFUNCTION()
	void HandleHPChanged(float Current, float Max);

	UFUNCTION()
	void HandleManaChanged(float Current, float Max);

	UPROPERTY()
	TObjectPtr<ALoLChampion> Champion;
};
