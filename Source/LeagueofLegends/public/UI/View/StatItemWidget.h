#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatItemWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class LEAGUEOFLEGENDS_API UStatItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateValue(float Value);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_StatValue;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_StatIcon;
};
