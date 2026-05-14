#include "GameFramework/PickWindowHUD.h"

#include "LeagueofLegends.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerState.h"
#include "Manager/ChampionDataSubsystem.h"
#include "UI/View/PickWindowWidget.h"
#include "UI/ViewModel/PickWindowViewModel.h"

void APickWindowHUD::BeginPlay()
{
	Super::BeginPlay();

	PRINTLOG_SH(TEXT("[PickWindowHUD] BeginPlay"));

	if (!PickWindowClass)
	{
		PRINTLOG_SH(TEXT("[PickWindowHUD] PickWindowClass is NULL"));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) { return; }

	auto* GS = GetWorld()->GetGameState<ARiftGameState>();
	auto* PS = PC->GetPlayerState<ARiftPlayerState>();
	auto* ChampSub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();

	UPickWindowViewModel* VM = NewObject<UPickWindowViewModel>(this);
	VM->Setup(GS, PS, ChampSub);

	// View 바인딩 먼저 → Initialize에서 OnChampionListReady 브로드캐스트 발생
	PickWindowWidget = CreateWidget<UPickWindowWidget>(PC, PickWindowClass);
	if (!PickWindowWidget)
	{
		PRINTLOG_SH(TEXT("[PickWindowHUD] CreateWidget 실패"));
		return;
	}

	PickWindowWidget->BindViewModel(VM);
	PickWindowWidget->AddToViewport();
	VM->Initialize();  // 구독 완료 후 브로드캐스트

	PRINTLOG_SH(TEXT("[PickWindowHUD] PickWindowWidget 생성 완료"));
}
