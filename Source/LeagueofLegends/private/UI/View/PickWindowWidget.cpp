#include "UI/View/PickWindowWidget.h"

#include "LeagueofLegends.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/LoLSessionSubsystem.h"
#include "Type/RiftTypes.h"
#include "UI/View/ChampSlotWidget.h"
#include "UI/View/TeamSlotWidget.h"
#include "UI/View/EnemySlotWidget.h"
#include "UI/ViewModel/PickWindowViewModel.h"

void UPickWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (btn_moveBlueTeam)
	{
		btn_moveBlueTeam->OnClicked.AddDynamic(this, &UPickWindowWidget::OnMoveBlueTeam);
	}

	if (btn_moveRedTeam)
	{
		btn_moveRedTeam->OnClicked.AddDynamic(this, &UPickWindowWidget::OnMoveRedTeam);
	}

	if (btn_Ready)
	{
		btn_Ready->OnClicked.AddDynamic(this, &UPickWindowWidget::OnReadyOrStart);
	}

	if (btn_quit)
	{
		btn_quit->OnClicked.AddDynamic(this, &UPickWindowWidget::OnQuit);
	}
}

void UPickWindowWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);

	if (auto* VM = Cast<UPickWindowViewModel>(InViewModel))
	{
		VM->OnPickWindowUpdated.AddUObject(this, &UPickWindowWidget::OnPickWindowUpdated);
		VM->OnChampionListReady.AddUObject(this, &UPickWindowWidget::OnChampionListReady);
		
		// 즉시 초기 로드 — OnReadyChanged 구독도 여기서 설정됨
		VM->RefreshFromGameState();
	}

	UpdateReadyButton();
}

void UPickWindowWidget::UnbindViewModel()
{
	if (auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel))
	{
		VM->OnPickWindowUpdated.RemoveAll(this);
		VM->OnChampionListReady.RemoveAll(this);
	}

	Super::UnbindViewModel();
}

void UPickWindowWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshAccum += InDeltaTime;
	if (RefreshAccum >= RefreshInterval)
	{
		RefreshAccum = 0.f;

		if (auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel))
		{
			VM->RefreshFromGameState();
		}
	}
}

void UPickWindowWidget::OnPickWindowUpdated()
{
	UpdateReadyButton();
	RefreshTeamSlots();
}

void UPickWindowWidget::OnChampionListReady(const TArray<FChampSlotViewData>& ChampList)
{
	if (!WrapBox_Champs || !ChampSlotClass) { return; }

	WrapBox_Champs->ClearChildren();

	for (const FChampSlotViewData& Data : ChampList)
	{
		UChampSlotWidget* ChampSlot = CreateWidget<UChampSlotWidget>(GetOwningPlayer(), ChampSlotClass);
		if (ChampSlot)
		{
			ChampSlot->SetChampionData(Data.ChampionID, Data.Portrait, Data.DisplayName);
			WrapBox_Champs->AddChildToWrapBox(ChampSlot);
		}
	}
}

void UPickWindowWidget::RefreshTeamSlots()
{
	auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel);
	if (!VM) return;

	// 블루팀 슬롯 (Spacer 있어서 인식 못하는 이슈: 슬롯 카운터 따로 관리)
	if (VB_BlueTeam)
	{
		TArray<FPlayerSlotViewData> BlueData = VM->GetBlueTeamPlayers();
		int32 SlotIdx = 0;
		for (int32 i = 0; i < VB_BlueTeam->GetChildrenCount(); i++)
		{
			if (auto* TeamSlot = Cast<UTeamSlotWidget>(VB_BlueTeam->GetChildAt(i)))
			{
				if (BlueData.IsValidIndex(SlotIdx))
				{
					TeamSlot->SetSlotData(BlueData[SlotIdx]);
				}
				else
				{
					TeamSlot->ClearSlot();
				}
				SlotIdx++;
			}
		}
	}

	// 레드팀 슬롯
	if (VB_RedTeam)
	{
		TArray<FPlayerSlotViewData> RedData = VM->GetRedTeamPlayers();
		int32 SlotIdx = 0;
		for (int32 i = 0; i < VB_RedTeam->GetChildrenCount(); i++)
		{
			if (auto* EnemySlot = Cast<UEnemySlotWidget>(VB_RedTeam->GetChildAt(i)))
			{
				if (RedData.IsValidIndex(SlotIdx))
				{
					EnemySlot->SetSlotData(RedData[SlotIdx]);
				}
				else
				{
					EnemySlot->ClearSlot();
				}
				SlotIdx++;
			}
		}
	}
}

void UPickWindowWidget::UpdateReadyButton()
{
	auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel);
	if (!VM) { return; }

	const bool bIsHost  = VM->IsLocalPlayerHost();
	const bool bIsReady = VM->IsLocalPlayerReady();

	if (bIsHost)
	{
		if (txt_ready_label)
			txt_ready_label->SetText(FText::FromString(TEXT("게임 시작")));
		if (btn_Ready)
			btn_Ready->SetIsEnabled(true);
	}
	else
	{
		if (bIsReady)
		{
			if (txt_ready_label)
			{
				txt_ready_label->SetText(FText::FromString(TEXT("준비 완료")));
			}
			if (btn_Ready)
			{
				btn_Ready->SetIsEnabled(false); // 중복 클릭 방지
			}
				
		}
		else
		{
			if (txt_ready_label)
			{
				txt_ready_label->SetText(FText::FromString(TEXT("준비")));
			}
				
			if (btn_Ready)
			{
				btn_Ready->SetIsEnabled(true);
			}
		}
	}
	
	/*auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel);
	if (!VM) { return; }

	const bool bIsHost = VM->IsLocalPlayerHost();

	// 호스트: "게임 시작" 버튼 활성 / 클라이언트: "준비 완료" 버튼, 게임 시작 불가
	if (btn_Ready)
	{
		btn_Ready->SetIsEnabled(true);
	}

	if (txt_ready_label)
	{
		txt_ready_label->SetText(
			bIsHost
				? FText::FromString(TEXT("Start Game"))
				: FText::FromString(TEXT("Ready"))
		);
	}*/
}

void UPickWindowWidget::OnMoveBlueTeam()
{
	if (auto* PC = Cast<ARiftPlayerController>(GetOwningPlayer()))
	{
		PC->Server_SelectTeam(ETeam::Blue);
	}
}

void UPickWindowWidget::OnMoveRedTeam()
{
	if (auto* PC = Cast<ARiftPlayerController>(GetOwningPlayer()))
	{
		PC->Server_SelectTeam(ETeam::Red);
	}
}

void UPickWindowWidget::OnReadyOrStart()
{
	PRINTLOG_SH(TEXT("[PickWindow] OnReadyOrStart 호출됨"));

	APlayerController* RawPC = GetOwningPlayer();
	PRINTLOG_SH(TEXT("[PickWindow] GetOwningPlayer = %s"), *GetNameSafe(RawPC));

	auto* PC = Cast<ARiftPlayerController>(RawPC);
	if (!PC)
	{
		PRINTLOG_SH(TEXT("[PickWindow] ARiftPlayerController 캐스팅 실패"));
		return;
	}

	auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel);
	const bool bIsHost = VM && VM->IsLocalPlayerHost();

	PRINTLOG_SH(TEXT("[PickWindow] IsHost=%d"), bIsHost);

	if (bIsHost)
	{
		PRINTLOG_SH(TEXT("[PickWindow] Server_StartGame 호출"));
		PC->Server_StartGame();
	}
	else
	{
		PRINTLOG_SH(TEXT("[PickWindow] Server_SetReady 호출"));
		PC->Server_SetReady();
	}
}

void UPickWindowWidget::OnQuit()
{
	auto* GI = GetGameInstance();
	if (GI)
	{
		if (auto* SessionSubsys = GI->GetSubsystem<ULoLSessionSubsystem>())
		{
			SessionSubsys->QuitSession();
		}
	}
}
