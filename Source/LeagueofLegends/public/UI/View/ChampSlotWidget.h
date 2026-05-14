#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChampSlotWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

UCLASS()
class LEAGUEOFLEGENDS_API UChampSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetChampionData(FName InChampionID, UTexture2D* Portrait, const FString& Name);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> btn_ChampPortrait;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Champ;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> txt_ChampName;

	UFUNCTION()
	void OnChampPortraitClicked();

	FName ChampionID = NAME_None;
};
