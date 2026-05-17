#include "UI/View/SkillBarWidget.h"

#include "LeagueofLegends.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Characters/Data/ChampionData.h"
#include "Manager/ChampionDataSubsystem.h"
#include "UI/View/SkillSlotWidget.h"
#include "UI/View/SpellSlotWidget.h"
#include "Components/CooldownComponent.h"
#include "UI/ViewModel/SkillBarViewModel.h"
#include "Components/StatComponent.h"
#include "GameFramework/RiftPlayerState.h"

void USkillBarWidget::BindViewModel(UViewModelBase* InViewModel)
{
	Super::BindViewModel(InViewModel);

	VM = Cast<USkillBarViewModel>(InViewModel);
	if (!VM) { return; }

	VM->OnHPChanged.AddUObject(this, &USkillBarWidget::OnHPChanged);
	VM->OnManaChanged.AddUObject(this, &USkillBarWidget::OnManaChanged);

	// 구독 후 현재값 즉시 반영
	if (UStatComponent* Stat = VM->GetStatComp())
	{
		OnHPChanged(Stat->GetCurrentHP(), Stat->GetMaxHP());
		OnManaChanged(Stat->GetCurrentMana(), Stat->GetMaxMana());
	}

	// 패시브 슬롯 
	if (Slot_P)
	{
		Slot_P->InitSlot(nullptr, NAME_None, FText::GetEmpty(), 0.f, 0.f);
		Slot_P->InitSlotMeta(ESkillSlot::Q, 0);

		// 패시브 슬롯만 크기 축소
		/*Slot_P->SetSlotSize(50.f, 50.f);
		Slot_P->SetPassive();*/
	}
	
	/*// 패시브 슬롯 — 레벨업 없음, 점 없음
	if (Slot_P)
	{
		Slot_P->InitSlot(nullptr, NAME_None, FText::FromString(TEXT("")), 0.f, 0.f);
		Slot_P->InitSlotMeta(ESkillSlot::Q, 0); // MaxRank=0 → 점/버튼 전부 숨김
	}*/
	
	// Q/W/E/R 슬롯
	UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	UChampionData* Data = VM->GetChampionData();
	const FName ChampID = Data ? Data->ChampionID : NAME_None;

	auto InitSkillSlot = [&](USkillSlotWidget* InSlot, FName CDTag, const FString& Key, const FString& Label)
	{
		if (!InSlot) { return; }
		float ManaCost = 0.f, MaxCD = 0.f;
		if (Sub && !ChampID.IsNone())
		{
			if (const FDetailSkillStatsRow* Row = Sub->GetSkillStats(ChampID, Key, 1))
			{
				ManaCost = Row->Cost;
				MaxCD = Row->CoolDown;
			}
		}
		InSlot->InitSlot(VM->GetCooldownComp(), CDTag, FText::FromString(Label), MaxCD, ManaCost);
	};

	InitSkillSlot(Slot_Q, TEXT("Skill.Q"), TEXT("Q"), TEXT("Q"));
	InitSkillSlot(Slot_W, TEXT("Skill.W"), TEXT("W"), TEXT("W"));
	InitSkillSlot(Slot_E, TEXT("Skill.E"), TEXT("E"), TEXT("E"));
	InitSkillSlot(Slot_R, TEXT("Skill.R"), TEXT("R"), TEXT("R"));

	// 슬롯 메타 초기화 (ESkillSlot, MaxRank)
	if (Slot_Q)
	{
		Slot_Q->InitSlotMeta(ESkillSlot::Q, 5);
	}
	
	if (Slot_W)
	{
		Slot_W->InitSlotMeta(ESkillSlot::W, 5);
	}
	
	if (Slot_E)
	{
		Slot_E->InitSlotMeta(ESkillSlot::E, 5);
	}
	
	if (Slot_R)
	{
		Slot_R->InitSlotMeta(ESkillSlot::R, 3);
	}

	// 레벨업 버튼/랭크 갱신 구독
	if (UStatComponent* Stat = VM->GetStatComp())
	{
		Stat->OnLevelChanged.AddUObject(this, &USkillBarWidget::OnLevelChanged);
	}
		

	if (USkillComponent* Skill = VM->GetSkillComp())
	{
		Skill->OnRankChanged.AddUObject(this, &USkillBarWidget::OnRankChanged);
	}
		

	RefreshSkillLevelUpButtons();

	if (Data)
	{
		if (Slot_P) { Slot_P->SetIcon(Data->PassiveIcon); }
		if (Slot_Q) { Slot_Q->SetIcon(Data->QIcon); }
		if (Slot_W) { Slot_W->SetIcon(Data->WIcon); }
		if (Slot_E) { Slot_E->SetIcon(Data->EIcon); }
		if (Slot_R) { Slot_R->SetIcon(Data->RIcon); }
	}
}

void USkillBarWidget::RefreshSkillLevelUpButtons()
{
	if (!VM) return;

	USkillComponent* SkillComp = VM->GetSkillComp();
	UStatComponent* StatComp = VM->GetStatComp();
	if (!SkillComp || !StatComp) { return; }

	PRINTLOG_SH(TEXT("[Skill] RefreshButtons Lv:%d Q:%d W:%d E:%d R:%d"),
	            StatComp->GetLevel(),
	            SkillComp->GetRank(ESkillSlot::Q), SkillComp->GetRank(ESkillSlot::W),
	            SkillComp->GetRank(ESkillSlot::E), SkillComp->GetRank(ESkillSlot::R));

	const int32 Level = StatComp->GetLevel();
	const int32 UsedPoints = SkillComp->GetRank(ESkillSlot::Q)
		+ SkillComp->GetRank(ESkillSlot::W)
		+ SkillComp->GetRank(ESkillSlot::E)
		+ SkillComp->GetRank(ESkillSlot::R);
	const int32 AvailablePoints = Level - UsedPoints;
	const bool bHasPoints = AvailablePoints > 0;

	auto Refresh = [&](USkillSlotWidget* InSlot, ESkillSlot SlotEnum, int32 Max, int32 MinLevel = 1)
	{
		if (!InSlot) { return; }
		const int32 Rank = SkillComp->GetRank(SlotEnum);
		const bool bLevelOk = Level >= MinLevel;
		InSlot->RefreshRank(Rank);
		InSlot->SetSkillActive(Rank > 0);
		InSlot->SetLevelUpAvailable(bHasPoints && Rank < Max && bLevelOk);
	};

	Refresh(Slot_Q, ESkillSlot::Q, 5);
	Refresh(Slot_W, ESkillSlot::W, 5);
	Refresh(Slot_E, ESkillSlot::E, 5);

	// R: 6/11/16 레벨에서만 찍을 수 있음 — 현재 랭크 기반으로 필요 레벨 결정
	if (Slot_R)
	{
		const int32 RRank = SkillComp->GetRank(ESkillSlot::R);
		constexpr int32 RLevelReq[] = {6, 11, 16};
		const bool bRLevelOk = RRank < 3 && Level >= RLevelReq[RRank];
		Slot_R->RefreshRank(RRank);
		Slot_R->SetSkillActive(RRank > 0);
		Slot_R->SetLevelUpAvailable(bHasPoints && bRLevelOk);
	}
}

void USkillBarWidget::OnLevelChanged(int32 /*NewLevel*/)
{
	RefreshSkillLevelUpButtons();
}

void USkillBarWidget::OnRankChanged()
{
	RefreshSkillLevelUpButtons();
}

void USkillBarWidget::InitSpellSlots(ARiftPlayerState* PS, UCooldownComponent* CD)
{
	if (!PS) { return; }
	if (Spell_D) { Spell_D->InitSlot(0, PS->GetSummonerSpell1()); }
	if (Spell_F) { Spell_F->InitSlot(1, PS->GetSummonerSpell2()); }
}

void USkillBarWidget::RefreshIcons(UChampionData* Data)
{
	if (!Data) { return; }
	if (Slot_P) { Slot_P->SetIcon(Data->PassiveIcon); }
	if (Slot_Q) { Slot_Q->SetIcon(Data->QIcon); }
	if (Slot_W) { Slot_W->SetIcon(Data->WIcon); }
	if (Slot_E) { Slot_E->SetIcon(Data->EIcon); }
	if (Slot_R) { Slot_R->SetIcon(Data->RIcon); }
}

static void SetBarPercent(UImage* BarImage, float Percent)
{
	if (!BarImage) { return; }
	BarImage->SetRenderTransformPivot(FVector2D(0.f, 0.5f));
	FWidgetTransform T;
	T.Scale = FVector2D(FMath::Clamp(Percent, 0.f, 1.f), 1.f);
	BarImage->SetRenderTransform(T);
}

void USkillBarWidget::OnHPChanged(float Current, float Max)
{
	SetBarPercent(Img_HP, Max > 0.f ? Current / Max : 0.f);
	if (Txt_HP)
	{
		Txt_HP->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
	}
}

void USkillBarWidget::OnManaChanged(float Current, float Max)
{
	SetBarPercent(Img_MP, Max > 0.f ? Current / Max : 0.f);
	if (Txt_MP)
	{
		Txt_MP->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
	}
}
