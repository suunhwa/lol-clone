#include "UI/View/ChampSlotWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "GameFramework/RiftPlayerController.h"

void UChampSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (btn_ChampPortrait)
		btn_ChampPortrait->OnClicked.AddDynamic(this, &UChampSlotWidget::OnChampPortraitClicked);
}

void UChampSlotWidget::SetChampionData(FName InChampionID, UTexture2D* Portrait, const FString& Name)
{
	ChampionID = InChampionID;

	if (txt_ChampName)
		txt_ChampName->SetText(FText::FromString(Name));

	if (Img_Champ && Portrait)
		Img_Champ->SetBrushFromTexture(Portrait);
}

void UChampSlotWidget::OnChampPortraitClicked()
{
	if (ChampionID == NAME_None) return;

	if (auto* PC = Cast<ARiftPlayerController>(GetOwningPlayer()))
		PC->Server_SelectChampion(ChampionID);
}
