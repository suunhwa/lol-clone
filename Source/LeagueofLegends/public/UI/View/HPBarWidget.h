#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPBarWidget.generated.h"

class UProgressBar;
class UStatComponent;

UCLASS()
class LEAGUEOFLEGENDS_API UHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void InitWidget(UStatComponent* InStatComp);

private:
	UFUNCTION()
	void OnHPChanged(float Current, float Max);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY()
	TObjectPtr<UStatComponent> StatComp;
};

