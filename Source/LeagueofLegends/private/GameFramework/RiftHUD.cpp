#include "GameFramework/RiftHUD.h"

#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/ItemDataSubsystem.h"
#include "UI/View/MainHUDWidget.h"
#include "UI/View/ShopWidget.h"
#include "UI/View/SkillBarWidget.h"
#include "UI/ViewModel/ShopViewModel.h"

ARiftHUD::ARiftHUD()
{
}

void ARiftHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ARiftHUD::InitHUD(ALoLChampion* Champion)
{
	// BP에서 이미 위젯을 만든 경우를 대비해 lazy 생성
	if (!MainHUDWidget && MainHUDClass)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			MainHUDWidget = CreateWidget<UMainHUDWidget>(PC, MainHUDClass);
			if (MainHUDWidget)
				MainHUDWidget->AddToViewport();
		}
	}

	PRINTLOG_SH(TEXT("[RiftHUD] InitHUD. Widget=%s Champion=%s"),
		*GetNameSafe(MainHUDWidget), *GetNameSafe(Champion));

	if (!MainHUDWidget || !Champion) return;

	APlayerController* PC = GetOwningPlayerController();
	ARiftPlayerState* PS = PC ? PC->GetPlayerState<ARiftPlayerState>() : nullptr;
	ARiftGameState*   GS = GetWorld()->GetGameState<ARiftGameState>();

	PRINTLOG_SH(TEXT("[RiftHUD] PS=%s GS=%s"), *GetNameSafe(PS), *GetNameSafe(GS));

	MainHUDWidget->InitHUD(Champion, PS, GS);

	PRINTLOG_TK(TEXT("Champion Components:"));
	for (UActorComponent* Comp : Champion->GetComponents())
	{
		PRINTLOG_TK(TEXT("  - %s"), *Comp->GetClass()->GetName());
	}
	
	SetupShopMVVM(Champion);
}

void ARiftHUD::RefreshSkillIcons(ALoLChampion* Champion)
{
	if (!MainHUDWidget || !Champion || !Champion->GetChampionData()) return;
	if (USkillBarWidget* Bar = MainHUDWidget->GetSkillBar())
		Bar->RefreshIcons(Champion->GetChampionData());
}

void ARiftHUD::ToggleShop()
{
	if (!ShopWidget) return;

	if (ShopWidget->IsVisible())
	{
		ShopWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		ShopWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ARiftHUD::SetupShopMVVM(ALoLChampion* Champion)
{
	if (!ShopWidgetClass)
	{
		PRINTLOG_TK(TEXT("SetupShopMVVM: ShopWidgetClass is not set"));
		return;
	}
	
	UInventoryComponent* InvComp = Champion->FindComponentByClass<UInventoryComponent>();
	UItemDataSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemDataSubsystem>();
	
	if (!InvComp || !ItemSubsystem)
	{
		PRINTLOG_TK(TEXT("SetupShopMVVM: Missing InventoryComponent or ItemDataSubsystem"));
		return;
	}
	
	// ViewModel 생성 (Initialize는 아직 호출하지 않음)
	ShopVM = NewObject<UShopViewModel>(this);
	ShopVM->SetItemDataSubsystem(ItemSubsystem);
	ShopVM->SetInventoryComponent(InvComp);
	
	// View를 먼저 생성하고 바인딩 → delegate가 등록된 상태에서 Initialize 호출
	APlayerController* PC = GetOwningPlayerController();
	ShopWidget = CreateWidget<UShopWidget>(PC, ShopWidgetClass);
	ShopWidget->BindViewModel(ShopVM);
	ShopWidget->AddToViewport();
	ShopWidget->SetVisibility(ESlateVisibility::Collapsed); // 처음에는 숨김

	// View 바인딩 완료 후 Initialize → BuildViewData → OnShopItemsBuilt.Broadcast 발생
	ShopVM->Initialize();
}





































