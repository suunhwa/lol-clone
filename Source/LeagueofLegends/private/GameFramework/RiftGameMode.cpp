#include "GameFramework/RiftGameMode.h"
#include "Type/RiftTypes.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftGameState.h"
#include "GameFramework/RiftPlayerController.h"
#include "GameFramework/RiftHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Characters/LoLPlayerStart.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "LeagueofLegends.h"
#include "Characters/LoLChampion.h"
#include "Manager/ChampionDataSubsystem.h"
#include "Struct/ExpStruct.h"
#include "Manager/MinionDataSubsystem.h"
#include "Struct/MinionStruct.h"
#include "Components/InventoryComponent.h"
#include "Manager/ObjectDataSubsystem.h"
#include "Characters/LoLChampionRespawnPoint.h"

ARiftGameMode::ARiftGameMode()
{
	PlayerStateClass = ARiftPlayerState::StaticClass();
	GameStateClass = ARiftGameState::StaticClass();
	PlayerControllerClass = ARiftPlayerController::StaticClass();
	HUDClass = ARiftHUD::StaticClass();
	DefaultPawnClass = ALoLChampion::StaticClass();
}

void ARiftGameMode::BeginPlay()
{
	Super::BeginPlay();
	CollectSpawnPoints();
}

void ARiftGameMode::CollectSpawnPoints()
{
	BlueSpawns.Reset();
	RedSpawns.Reset();

	for (TActorIterator<ALoLPlayerStart> It(GetWorld()); It; ++It)
	{
		if (It->Team == ETeam::Blue)
		{
			BlueSpawns.Add(*It);
		}
		else if (It->Team == ETeam::Red)
		{
			RedSpawns.Add(*It);
		}
	}

	BlueSpawns.Sort([](const ALoLPlayerStart& A, const ALoLPlayerStart& B) { return A.SlotIndex < B.SlotIndex; });
	RedSpawns.Sort([](const ALoLPlayerStart& A, const ALoLPlayerStart& B) { return A.SlotIndex < B.SlotIndex; });

	PRINTLOG_SH(TEXT("[SpawnPoints] Blue:%d Red:%d"), BlueSpawns.Num(), RedSpawns.Num());

	for (ALoLPlayerStart* S : BlueSpawns)
	{
		if (S)
		{
			PRINTLOG_SH(TEXT("  [Blue] Slot%d → %s  (%.0f, %.0f, %.0f)"),
			            S->SlotIndex,
			            *GetNameSafe(S),
			            S->GetActorLocation().X,
			            S->GetActorLocation().Y,
			            S->GetActorLocation().Z);
		}
	}
	for (ALoLPlayerStart* S : RedSpawns)
	{
		if (S)
		{
			PRINTLOG_SH(TEXT("  [Red]  Slot%d → %s  (%.0f, %.0f, %.0f)"),
			            S->SlotIndex,
			            *GetNameSafe(S),
			            S->GetActorLocation().X,
			            S->GetActorLocation().Y,
			            S->GetActorLocation().Z);
		}
	}
}

void ARiftGameMode::AssignTeamSlot(ARiftPlayerState* PS)
{
	if (!PS || !GameState) { return; }

	TSet<int32> UsedSlots;
	for (APlayerState* OtherPS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
		{
			if (RPS != PS && RPS->GetTeam() == PS->GetTeam() && RPS->GetTeamSlotIndex() != INDEX_NONE)
			{
				UsedSlots.Add(RPS->GetTeamSlotIndex());
			}
		}
	}

	for (int32 i = 0; i < 5; ++i)
	{
		if (!UsedSlots.Contains(i))
		{
			PS->SetTeamSlotIndex(i);
			return;
		}
	}
}

void ARiftGameMode::PostLogin(APlayerController* NewPlayer)
{
	// Super::PostLogin 안에서 HandleStartingNewPlayer → RestartPlayer → ChoosePlayerStart가 호출되므로
	// 팀/슬롯 배정은 반드시 Super 호출 전에 완료해야 함
	ARiftPlayerState* PS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (PS)
	{
		// Seamless Travel 실패(PIE 등)로 PlayerState 초기화된 경우 팀 재배정
		if (PS->GetTeam() == ETeam::None)
		{
			int32 Blue = 0, Red = 0;
			for (APlayerState* OtherPS : GameState->PlayerArray)
			{
				if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
				{
					if (RPS != PS)
					{
						if (RPS->GetTeam() == ETeam::Blue) Blue++;
						else if (RPS->GetTeam() == ETeam::Red) Red++;
					}
				}
			}
			PS->SetTeam(Blue <= Red ? ETeam::Blue : ETeam::Red);
			PRINTLOG_SH(TEXT("[PostLogin] Team 재배정 (SeamlessTravel 미작동) → %s"),
			            PS->GetTeam() == ETeam::Blue ? TEXT("Blue") : TEXT("Red"));
		}

		if (PS->GetTeamSlotIndex() == INDEX_NONE)
		{
			AssignTeamSlot(PS);
		}

		PRINTLOG_SH(TEXT("[PostLogin] Team:%s Slot:%d (Super 호출 전)"),
		            PS->GetTeam() == ETeam::Blue ? TEXT("Blue") : TEXT("Red"),
		            PS->GetTeamSlotIndex());
	}

	Super::PostLogin(NewPlayer); // 이 안에서 ChoosePlayerStart 호출됨

	TryStartGame();
}

void ARiftGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// CopyProperties로 PickWindow 팀/슬롯이 넘어왔어야 함.
	// CopyProperties 미동작 등 예외 상황의 폴백 처리
	ARiftPlayerState* PS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (PS)
	{
		PRINTLOG_SH(TEXT("[HandleStartingNewPlayer] Before Team=%d Slot=%d Name=%s"),
		            (int32)PS->GetTeam(),
		            PS->GetTeamSlotIndex(),
		            *PS->GetPlayerName());

		if (PS->GetTeam() == ETeam::None)
		{
			// CopyProperties 정상이면 이 로그 안 찍혀야 함
			PRINTLOG_SH(TEXT("[HandleStartingNewPlayer] ★ Team=None — CopyProperties 미동작"));
			int32 Blue = 0, Red = 0;
			for (APlayerState* OtherPS : GameState->PlayerArray)
			{
				if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
				{
					if (RPS != PS)
					{
						if (RPS->GetTeam() == ETeam::Blue) Blue++;
						else if (RPS->GetTeam() == ETeam::Red) Red++;
					}
				}
			}
			PS->SetTeam(Blue <= Red ? ETeam::Blue : ETeam::Red);
		}

		if (PS->GetTeamSlotIndex() == INDEX_NONE)
		{
			AssignTeamSlot(PS);
		}

		PRINTLOG_SH(TEXT("[HandleStartingNewPlayer] After  Team=%d Slot=%d → 스폰"),
		            (int32)PS->GetTeam(),
		            PS->GetTeamSlotIndex());
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer); // → RestartPlayer → ChoosePlayerStart

	// 스폰 직후 지형 위로 즉시 내려붙임 (중력 낙하 없이 바로 정확한 위치에서 시작)
	if (ACharacter* SpawnedChar = Cast<ACharacter>(NewPlayer->GetPawn()))
	{
		const FVector StartTrace = SpawnedChar->GetActorLocation() + FVector(0.f, 0.f, 200.f);
		const FVector EndTrace   = SpawnedChar->GetActorLocation() - FVector(0.f, 0.f, 500.f);

		FHitResult GroundHit;
		FCollisionQueryParams Params(NAME_None, false, SpawnedChar);
		if (GetWorld()->LineTraceSingleByChannel(GroundHit, StartTrace, EndTrace, ECC_WorldStatic, Params))
		{
			const float HalfHeight = SpawnedChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			FVector GroundLoc = GroundHit.ImpactPoint + FVector(0.f, 0.f, HalfHeight);
			SpawnedChar->SetActorLocation(GroundLoc, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	// 스폰 완료 후 PlayerState의 SelectedChampionID로 ChampionData 주입
	ALoLChampion* Champ = Cast<ALoLChampion>(NewPlayer->GetPawn());
	ARiftPlayerState* RPS = NewPlayer->GetPlayerState<ARiftPlayerState>();
	if (Champ && RPS)
	{
		const FName ChampionID = RPS->GetSelectedChampion();
		if (!ChampionID.IsNone())
		{
			UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
			if (UChampionData* Data = Sub ? Sub->GetChampionData(ChampionID) : nullptr)
			{
				Champ->SetChampionData(Data);
				PRINTLOG_SH(TEXT("[HandleStartingNewPlayer] ChampionData 주입: %s → %s"),
					*RPS->GetPlayerName(), *ChampionID.ToString());
			}
			else
			{
				PRINTLOG_SH(TEXT("[HandleStartingNewPlayer] ChampionData 없음 (ID: %s) — Blueprint 기본값 사용"),
					*ChampionID.ToString());
			}
		}
	}
}

void ARiftGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ARiftPlayerState* ps = Exiting->GetPlayerState<ARiftPlayerState>();
	if (!ps) { return; }

	ps->SetDisconnected(true);
}

AActor* ARiftGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	ARiftPlayerState* PS = Player ? Player->GetPlayerState<ARiftPlayerState>() : nullptr;
	if (!PS)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const bool bBlue = (PS->GetTeam() == ETeam::Blue);

	// 스폰 배열 비어있으면 즉시 수집
	if (BlueSpawns.IsEmpty() && RedSpawns.IsEmpty())
	{
		PRINTLOG_SH(TEXT("[ChoosePlayerStart] 스폰 배열 비어있음 — 즉시 재수집"));
		CollectSpawnPoints();
	}

	TArray<TObjectPtr<ALoLPlayerStart>>& Spawns = bBlue ? BlueSpawns : RedSpawns;

	if (!Spawns.IsEmpty())
	{
		const int32 SlotIndex = PS->GetTeamSlotIndex();
		for (ALoLPlayerStart* S : Spawns)
		{
			if (S && S->SlotIndex == SlotIndex)
			{
				PRINTLOG_SH(TEXT("[ChoosePlayerStart] %s팀 Slot%d → %s  위치:(%.0f, %.0f, %.0f)"),
				            bBlue ? TEXT("Blue") : TEXT("Red"),
				            SlotIndex,
				            *GetNameSafe(S),
				            S->GetActorLocation().X,
				            S->GetActorLocation().Y,
				            S->GetActorLocation().Z);
				return S;
			}
		}
		PRINTLOG_SH(TEXT("[ChoosePlayerStart] %s팀 Slot%d 없음 — Spawns[0] 폴백"),
		            bBlue ? TEXT("Blue") : TEXT("Red"),
		            SlotIndex);
		return Spawns[0];
	}

	// ── ALoLPlayerStart 미배치 시 기존 태그 기반 폴백 ───────────────────────
	// if (PS->GetTeam() == ETeam::None)
	// {
	// 	int32 Blue = 0, Red = 0;
	// 	for (APlayerState* OtherPS : GameState->PlayerArray)
	// 		if (auto* RPS = Cast<ARiftPlayerState>(OtherPS))
	// 		{
	// 			if (RPS->GetTeam() == ETeam::Blue) Blue++;
	// 			else if (RPS->GetTeam() == ETeam::Red) Red++;
	// 		}
	// 	PS->SetTeam(Blue <= Red ? ETeam::Blue : ETeam::Red);
	// }
	// FName Tag = (PS->GetTeam() == ETeam::Blue) ? FName("BlueTeam") : FName("RedTeam");
	// TArray<APlayerStart*> ValidStarts;
	// for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	// {
	// 	if (It->PlayerStartTag == Tag) ValidStarts.Add(*It);
	// }
	// if (!ValidStarts.IsEmpty())
	// 	return ValidStarts[FMath::RandRange(0, ValidStarts.Num() - 1)];
	// ────────────────────────────────────────────────────────────────────────

	PRINTLOG_SH(TEXT("[ChoosePlayerStart] ALoLPlayerStart 없음 — 맵에 배치 필요"));
	return Super::ChoosePlayerStart_Implementation(Player);
}

void ARiftGameMode::OnNexusDestroyed(ETeam DestroyedTeam)
{
	ETeam Winner = (DestroyedTeam == ETeam::Blue) ? ETeam::Red : ETeam::Blue;
	EndGame(Winner);
}

void ARiftGameMode::OnChampionKilled(ARiftPlayerState* Killer,
                                     ARiftPlayerState* Victim,
                                     const TArray<ARiftPlayerState*>& Assisters)
{
	if (!Victim) { return; }
	Victim->AddDeath();

	// 죽은 챔피언 위치 확보 + 부활 타이머 시작
	FVector VictimLocation = FVector::ZeroVector;
	if (APawn* VictimPawn = Victim->GetPawn())
	{
		VictimLocation = VictimPawn->GetActorLocation();

		if (ALoLChampion* VictimChamp = Cast<ALoLChampion>(VictimPawn))
		{
			const ARiftGameState* GS = GetGameState<ARiftGameState>();
			const float GameSecs = GS ? static_cast<float>(GS->GetElapsedSeconds()) : 0.f;
			const int32 RespawnSecs = CalculateRespawnTime(Victim->GetChampionLevel(), GameSecs);
			VictimChamp->StartRespawnTimer(static_cast<float>(RespawnSecs));

			// 사망 상태 + 부활 종료 시간 PlayerState에 복제
			const AGameStateBase* GSBase = GetGameState<AGameStateBase>();
			const float EndServerTime = GSBase
				? GSBase->GetServerWorldTimeSeconds() + static_cast<float>(RespawnSecs)
				: 0.f;
			Victim->SetDeadState(true, EndServerTime);
		}
	}
	
	if (!Killer || Killer == Victim) { return; }
	Killer->AddKill();

	for (ARiftPlayerState* A : Assisters)
	{
		if (A && A != Killer)
		{
			A->AddAssist();
		}
	}

	ARiftGameState* GS = GetGameState<ARiftGameState>();
	if (GS)
	{
		GS->AddTeamKill(Killer->GetTeam());
	}

	// XP 분배
	UChampionDataSubsystem* ExpSys = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
	if (!ExpSys) { return; }

	const FChampionKillExpRow* Row = ExpSys->GetChampionKillExpRow(Victim->GetChampionLevel());
	if (!Row) { return; }

	// 킬러 + 어시스터를 하나의 수령자 목록으로 합산
	TArray<ARiftPlayerState*> Participants = {Killer};
	for (ARiftPlayerState* A : Assisters)
	{
		if (A && A != Killer)
		{
			Participants.Add(A);
		}
	}

	for (ARiftPlayerState* Participant : Participants)
	{
		if (!Participant) { continue; }
		// 레벨 보정 → 참여자 수로 균등 분배
		float XP = CalcChampionKillXP(Row->RewardXP, Participant->GetChampionLevel(), Victim->GetChampionLevel());
		// 💡 이 부분 수정: 균등 분배된 실제 지급용 XP 변수화 및 UI 호출
		float DistributedXP = XP / Participants.Num();
		Participant->AddXP(DistributedXP);
		
		// 🔮 [경험치] 획득한 각 챔피언 자신의 머리 위에 팝업
		if (ALoLCharacterBase* ChampPawn = Cast<ALoLCharacterBase>(Participant->GetPawn()))
		{
			FVector ChampHeadLoc = ChampPawn->GetActorLocation() + FVector(0.f, 0.f, 180.f);
			ChampPawn->Client_CreateFloatingText(FMath::RoundToInt(DistributedXP), false, ChampHeadLoc);
		}
	}
	
	
	// 챔피언 킬/어시스트 골드 정산 파트 
	
	// 제압 킬 등을 위해 나중에 테이블 연동이 가능하도록 변수화
	int32 BaseKillGold = 300; 
	int32 TotalAssistPool = BaseKillGold / 2; // 어시스트 총 풀은 킬 값의 50% (150원)

	// 킬러(막타자) 골드 지급
	if (APawn* KillerPawn = Killer->GetPawn())
	{
		if (UInventoryComponent* KillerInv = KillerPawn->FindComponentByClass<UInventoryComponent>())
		{
			KillerInv->AddGold(static_cast<float>(BaseKillGold));
            
			// 💰 [골드] 킬러 머리 위가 아니라, '죽은 상대방 챔피언 위치' 허공에 고정 팝업!
			if (ALoLCharacterBase* KillerChampBase = Cast<ALoLCharacterBase>(KillerPawn))
			{
				FVector GoldSpawnLoc = VictimLocation + FVector(0.f, 0.f, 120.f);
				KillerChampBase->Client_CreateFloatingText(BaseKillGold, true, GoldSpawnLoc);
			}
			PRINTLOG_HJ(TEXT("[Champion Kill Gold] %s ➔ 상대 %s 처치! +%d Gold 주입 (잔액: %.0f)"), 
			   *Killer->GetPlayerName(), *Victim->GetPlayerName(), BaseKillGold, KillerInv->GetGold());
		}
	}

	// 어시스터 유효 인원 걸러내기 
	TArray<ARiftPlayerState*> ValidAssisters;
	for (ARiftPlayerState* A : Assisters)
	{
		if (A && A != Killer)
		{
			ValidAssisters.Add(A);
		}
	}

	// 어시스터 골드 1/N 분배 지급
	if (!ValidAssisters.IsEmpty())
	{
		// 총 어시스트 골드(150원)를 참여한 아군 수로 쪼개서 지급
		int32 AssistGoldEach = TotalAssistPool / ValidAssisters.Num();

		for (ARiftPlayerState* AssisterPS : ValidAssisters)
		{
			if (APawn* AssisterPawn = AssisterPS->GetPawn())
			{
				if (UInventoryComponent* AssisterInv = AssisterPawn->FindComponentByClass<UInventoryComponent>())
				{
					AssisterInv->AddGold(static_cast<float>(AssistGoldEach));
                    
					// 💰 [골드] 어시스터들도 마찬가지로 '죽은 상대방 챔피언 위치'에서 돈다발이 연출됨
					if (ALoLCharacterBase* AssisterChampBase = Cast<ALoLCharacterBase>(AssisterPawn))
					{
						FVector GoldSpawnLoc = VictimLocation + FVector(0.f, 0.f, 140.f); // 약간 겹치지 않게 오프셋 조절 가능
						AssisterChampBase->Client_CreateFloatingText(AssistGoldEach, true, GoldSpawnLoc);
					}
					PRINTLOG_HJ(TEXT("[Champion Assist Gold] %s ➔ 처치 지원 보너스 +%d Gold 주입 (잔액: %.0f)"), 
						*AssisterPS->GetPlayerName(), AssistGoldEach, AssisterInv->GetGold());
				}
			}
		}
	}
}

void ARiftGameMode::OnUnitKilled(FName UnitRowName, FVector KillLocation, ETeam KillerTeam, AActor* DamageInstigator)
{
    UChampionDataSubsystem* ExpSys = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>();
    if (!ExpSys) { return; }

    const FUnitRewardExpRow* Row = ExpSys->GetUnitRewardRow(UnitRowName);
    if (!Row) { return; }

    ARiftGameState* GS = GetGameState<ARiftGameState>();
    float GameMinutes = GS ? GS->GetElapsedSeconds() / 60.0f : 0.0f;

    float UnitXP = CalcUnitXP(*Row, GameMinutes);
    TArray<ARiftPlayerState*> Nearby = FindNearbyAllies(KillLocation, Row->ExpRadius, KillerTeam);
    
    // [1] 경험치 정산 파트 (기존 코드 유지)
    if (!Nearby.IsEmpty())
    {	
       float XPEach = Nearby.Num() == 1 ? UnitXP : UnitXP * Row->SharingMultiplier / Nearby.Num();
       for (ARiftPlayerState* PS : Nearby)
       {
          PS->AddXP(XPEach);

       	// 🔮 [경험치] 주변에서 짭짤하게 나눠 먹은 아군들 각각의 정수리 위 팝업
       	if (ALoLCharacterBase* AllyChamp = Cast<ALoLCharacterBase>(PS->GetPawn()))
       	{
       		FVector ChampHeadLoc = AllyChamp->GetActorLocation() + FVector(0.f, 0.f, 180.f);
       		AllyChamp->Client_CreateFloatingText(FMath::RoundToInt(XPEach), false, ChampHeadLoc);
       	}
       }
    }
	
    // 골드 정산 파트 (구조체 필드 및 글로벌 골드 규칙 완벽 매칭)
    if (IsValid(DamageInstigator))
    {
        if (ALoLChampion* KillerChamp = Cast<ALoLChampion>(DamageInstigator))
        {
            if (UInventoryComponent* InvComp = KillerChamp->FindComponentByClass<UInventoryComponent>())
            {
                int32 FinalLastHitGold = 0;
                int32 FinalGlobalGold = 0;
                bool bShouldDistributeGlobal = false;
                FString RowNameStr = UnitRowName.ToString();

                // ─── 죽은 오브젝트가 구조물(포탑, 억제기, 넥서스) 계열일 때 ───
                if (RowNameStr.Contains(TEXT("Tower")) || RowNameStr.Equals(TEXT("Inhibitor")) || RowNameStr.Equals(TEXT("Nexus")))
                {
                    UObjectDataSubsystem* ObjectDataSub = GetGameInstance()->GetSubsystem<UObjectDataSubsystem>();
                    if (ObjectDataSub)
                    {
                        FObjectBaseRow StructureBaseData;
                        FObjectRewardRow StructureRewardData;
                        FObjectMechanicsRow StructureMechData;

                        // FName을 기반으로 ObjectID 정확하게 분기 매핑
                        int32 TargetObjectID = 11001; 
                        if (UnitRowName == FName(TEXT("Tower_Outer")))          TargetObjectID = 11001;
                        else if (UnitRowName == FName(TEXT("Tower_Inner")))     TargetObjectID = 11002;
                        else if (UnitRowName == FName(TEXT("Tower_Inhibitor"))) TargetObjectID = 11003;
                        else if (UnitRowName == FName(TEXT("Tower_Nexus")))     TargetObjectID = 11004;
                        else if (UnitRowName == FName(TEXT("Inhibitor")))       TargetObjectID = 11101;
                        else if (UnitRowName == FName(TEXT("Nexus")))           TargetObjectID = 11111;

                        // 서브시스템에서 캐싱된 맵 데이터를 읽어옵니다.
                        if (ObjectDataSub->GetAllTowerData(TargetObjectID, StructureBaseData, StructureRewardData, StructureMechData))
                        {
                            // 시트 컬럼명과 완벽 동기화
                            FinalLastHitGold = FMath::RoundToInt(StructureRewardData.Last_Hit_Gold);
                            FinalGlobalGold = FMath::RoundToInt(StructureRewardData.Global_Gold);
                            bShouldDistributeGlobal = StructureRewardData.bGlobalDist;

                            PRINTLOG_HJ(TEXT("[GameMode 구조물] %s 데이터 로드 성공 ➔ 막타: %d원, 글로벌: %d원 (글로벌 분배 여부: %s)"), 
                                *RowNameStr, FinalLastHitGold, FinalGlobalGold, bShouldDistributeGlobal ? TEXT("True") : TEXT("False"));
                        }
                        else
                        {
                            FinalLastHitGold = 250; // 예외 폴백값
                            FinalGlobalGold = 0;
                        }
                    }
                }
                // ─── 미니언 계열일 때 ───
                else
                {
                    UMinionDataSubsystem* MinionDataSub = GetGameInstance()->GetSubsystem<UMinionDataSubsystem>();
                    if (MinionDataSub)
                    {
                        int32 TargetMinionID = 0;
                        if (UnitRowName == FName(TEXT("Minion_Melee")))       TargetMinionID = 3001;
                        else if (UnitRowName == FName(TEXT("Minion_Ranged"))) TargetMinionID = 3002;
                        else if (UnitRowName == FName(TEXT("Minion_Siege")))  TargetMinionID = 3003;
                        else if (UnitRowName == FName(TEXT("Minion_Super")))  TargetMinionID = 3004;

                        if (FMinionGrowthRow* GrowthRow = MinionDataSub->GetGrowthRowByID(TargetMinionID))
                        {
                            float BaseGold = static_cast<float>(GrowthRow->Base_Gold);
                            float GoldUp = static_cast<float>(GrowthRow->Gold_Up);
                            FinalLastHitGold = FMath::RoundToInt(BaseGold + (GoldUp * GameMinutes));
                        }
                        else { FinalLastHitGold = 21; }
                    }
                }

            	// 막타 챔피언 인벤토리 정산 및 UI 호출
            	if (FinalLastHitGold > 0)
            	{
            		InvComp->AddGold(static_cast<float>(FinalLastHitGold));
                    
            		// 💰 [골드] 막타 친 챔피언 머리가 아니라 인자로 넘어온 미니언/타워의 'KillLocation'에 소환!
            		FVector GoldSpawnLoc = KillLocation + FVector(0.f, 0.f, 120.f); 
            		KillerChamp->Client_CreateFloatingText(FinalLastHitGold, true, GoldSpawnLoc);
                    
            		PRINTLOG_HJ(TEXT("[Reward System] %s 막타 보상 ➔ +%d Gold 주입 완료 (현재 보유: %.0f)"), 
						*UnitRowName.ToString(), FinalLastHitGold, InvComp->GetGold());
            	}

                // bGlobalDist가 켜져 있으면 같은 팀원 전체에게 보너스 골드 지급
                if (bShouldDistributeGlobal && FinalGlobalGold > 0 && IsValid(GameState))
                {
                    for (APlayerState* PS : GameState->PlayerArray)
                    {
                        if (ARiftPlayerState* RPS = Cast<ARiftPlayerState>(PS))
                        {
                            // 막타 친 팀원과 같은 팀 소속인 아군 플레이어들에게만 골드 선물
                            if (RPS->GetTeam() == KillerTeam)
                            {
                                if (APawn* AlliedPawn = RPS->GetPawn())
                                {
                                    if (UInventoryComponent* AlliedInv = AlliedPawn->FindComponentByClass<UInventoryComponent>())
                                    {
                                        AlliedInv->AddGold(static_cast<float>(FinalGlobalGold));
                                        
                                    	// 💰 [글로벌 골드] 맵 전체 아군에게 보낼 때도 포탑이 터진 'KillLocation' 좌표에 띄웁니다!
                                    	// 맵 어디에 있든 화면(Screen) 기준 오브젝트 위치에서 돈 오르는 연출이 발생합니다.
                                    	if (ALoLCharacterBase* AlliedChampBase = Cast<ALoLCharacterBase>(AlliedPawn))
                                    	{
                                    		FVector GlobalGoldSpawnLoc = KillLocation + FVector(0.f, 0.f, 150.f); // 막타 텍스트와 겹치지 않게 살짝 위로 오프셋
                                    		AlliedChampBase->Client_CreateFloatingText(FinalGlobalGold, true, GlobalGoldSpawnLoc);
                                    	}
                                    	PRINTLOG_HJ(TEXT("[Global Reward] 구조물 파괴 보너스 ➔ %s 플레이어에게 +%d 글로벌 골드 지급 완료!"), 
											*RPS->GetPlayerName(), FinalGlobalGold);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
	
}

float ARiftGameMode::CalcChampionKillXP(float BaseXP, int32 KillerLevel, int32 VictimLevel)
{
	int32 LevelDiff = VictimLevel - KillerLevel; // 양수: 적이 더 높음, 음수: 적이 더 낮음
	float Multiplier;
	if (LevelDiff > 0)
	{
		Multiplier = 1.0f + LevelDiff * 0.16f;
	}
	else if (LevelDiff < 0)
	{
		// 하한 20% 보장
		Multiplier = FMath::Max(0.2f, 1.0f + LevelDiff * 0.10f);
	}
	else
	{
		Multiplier = 1.0f;
	}

	return BaseXP * Multiplier;
}

float ARiftGameMode::CalcUnitXP(const struct FUnitRewardExpRow& Row, float GameMinutes)
{
	return FMath::Min(Row.BaseXP + Row.GrowthPerMinute * GameMinutes, Row.MaxXP);
}

int32 ARiftGameMode::CalculateRespawnTime(int32 ChampionLevel, float GameTimeSeconds, float AdditionalRespawnTime) const
{
	// 1. 레벨별 기본 부활 시간 (DataTable)
	float BaseTime = 6.f; // 폴백
	if (UChampionDataSubsystem* Sub = GetGameInstance()->GetSubsystem<UChampionDataSubsystem>())
	{
		if (const FChampionRespawnRow* Row = Sub->GetRespawnRow(ChampionLevel))
		{
			BaseTime = Row->Respawn_Time_Base;
		}
	}

	// 2. 시간 보정치: 31분부터 매 완전한 1분마다 +0.02, 최대 1.5
	const float GameMinutes = GameTimeSeconds / 60.f;
	float Modifier = 1.f;
	if (GameMinutes >= 31.f)
	{
		const float ExtraMinutes = FMath::FloorToFloat(GameMinutes - 30.f);
		Modifier += ExtraMinutes * 0.02f;
		Modifier = FMath::Min(Modifier, 1.5f);
	}

	// 3. 최종 계산 + 반올림
	const float FinalTime = (BaseTime * Modifier) + AdditionalRespawnTime;
	return FMath::RoundToInt(FinalTime);
}

ALoLChampionRespawnPoint* ARiftGameMode::FindRespawnPoint(ETeam Team) const
{
	for (TActorIterator<ALoLChampionRespawnPoint> It(GetWorld()); It; ++It)
	{
		if (It->Team == Team)
		{
			return *It;
		}
	}
	return nullptr;
}

void ARiftGameMode::RespawnChampion(ALoLChampion* Champion)
{
	if (!Champion || !Champion->HasAuthority()) { return; }

	// 팀 + 슬롯 기반 부활 위치 결정
	const ARiftPlayerState* PS = Champion->GetPlayerState<ARiftPlayerState>();
	const ETeam MyTeam = PS ? PS->GetTeam() : ETeam::None;
	ALoLChampionRespawnPoint* Point = FindRespawnPoint(MyTeam);

	if (Point)
	{
		const int32 SlotIndex = PS ? PS->GetTeamSlotIndex() : 0;
		const int32 Idx = SlotIndex % ALoLChampionRespawnPoint::SlotOffsets.Num();
		const FVector SpawnLoc = Point->GetActorLocation() + ALoLChampionRespawnPoint::SlotOffsets[Idx];

		// TeleportTo 대신 직접 설정 (충돌 실패 없음)
		Champion->SetActorLocationAndRotation(SpawnLoc, Point->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
	}

	Champion->Respawn();
}

TArray<ARiftPlayerState*> ARiftGameMode::FindNearbyAllies(FVector Location, float Radius, ETeam Team) const
{
	TArray<ARiftPlayerState*> Result;
	for (APlayerState* PS : GameState->PlayerArray)
	{
		ARiftPlayerState* RPS = Cast<ARiftPlayerState>(PS);
		if (!RPS || RPS->GetTeam() != Team) { continue; }

		APawn* Pawn = RPS->GetPawn();
		if (Pawn && FVector::Dist(Pawn->GetActorLocation(), Location) <= Radius)
		{
			Result.Add(RPS);
		}
	}
	return Result;
}

/*void ARiftGameMode::OnChampionKilled(ARiftPlayerState* Killer, ARiftPlayerState* Victim)
{
	if (!Victim) { return; }
	Victim->AddDeath();

	if (!Killer || Killer == Victim) { return; }
	Killer->AddKill();

	ARiftGameState* GS = GetGameState<ARiftGameState>();
	if (!GS) { return; }
	GS->AddTeamKill(Killer->GetTeam());
}*/

void ARiftGameMode::TryStartGame()
{
	int32 Blue = 0, Red = 0;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (auto* RPS = Cast<ARiftPlayerState>(PS))
		{
			if (RPS->GetTeam() == ETeam::Blue)
			{
				Blue++;
			}
			else if (RPS->GetTeam() == ETeam::Red)
			{
				Red++;
			}
		}
	}

	if (Blue >= PlayersPerTeam && Red >= PlayersPerTeam)
	{
		StartGame();
	}
}

void ARiftGameMode::StartGame()
{
	ARiftGameState* gs = GetGameState<ARiftGameState>();
	if (!gs) { return; }

	gs->SetPhase(EGamePhase::InGame);
	gs->StartGameTimer();
}

void ARiftGameMode::EndGame(ETeam WinningTeam)
{
	ARiftGameState* gs = GetGameState<ARiftGameState>();
	if (!gs) { return; }

	gs->SetPhase(EGamePhase::GameOver);
	gs->SetWinningTeam(WinningTeam);
	gs->StopGameTimer();
}
