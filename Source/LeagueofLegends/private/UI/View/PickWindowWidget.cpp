#include "UI/View/PickWindowWidget.h"

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
	}
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
	auto* PC = Cast<ARiftPlayerController>(GetOwningPlayer());
	if (!PC) { return; }

	auto* VM = Cast<UPickWindowViewModel>(OwnerViewModel);
	if (VM && VM->IsLocalPlayerHost())
	{
		PC->Server_StartGame();
	}

	else
	{
		PC->Server_SetReady();
	}
}

void UPickWindowWidget::OnQuit()
{
	auto* GI = GetGameInstance();
	if (!GI) { return; }

	if (auto* SessionSubsys = GI->GetSubsystem<ULoLSessionSubsystem>())
	{
		SessionSubsys->QuitSession();
	}
}
