#include "GameFramework/RiftPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "LeagueofLegends.h"
#include "AStar/AStarGridManager.h"
#include "Engine/LocalPlayer.h"
#include "Characters/LoLChampion.h"
#include "Characters/LoLCharacterBase.h"
#include "GameFramework/RiftPlayerCameraManager.h"
#include "GameFramework/PickWindowGameMode.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftHUD.h"
#include "Components/CombatComponent.h"
#include "Components/StatComponent.h"
#include "GameFramework/LoLGameInstance.h"
#include "GameFramework/RiftGameState.h"
#include "Components/CooldownComponent.h"
#include "Components/StatComponent.h"
#include "Struct/SpellStruct.h"
#include "UI/View/MainHUDWidget.h"
#include "UI/View/SkillBarWidget.h"
#include "UI/View/SpellSlotWidget.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/Targetable.h"
#include "Kismet/GameplayStatics.h"

ARiftPlayerController::ARiftPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
	PlayerCameraManagerClass = ARiftPlayerCameraManager::StaticClass();
}

void ARiftPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) { return; }

	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);

	if (auto* GI = GetGameInstance<ULoLGameInstance>())
	{
		if (!GI->Nickname.IsEmpty())
		{
			ServerChangeName(GI->Nickname);
		}
	}

	// A* 로직 완성되면
	// GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void ARiftPlayerController::SeamlessTravelTo(APlayerController* NewPC)
{
	Super::SeamlessTravelTo(NewPC);

	// Seamless Travel 완료 후 닉네임 재전송 (BeginPlay는 재호출 안 되므로)
	if (!IsLocalController()) { return; }

	if (auto* GI = GetGameInstance<ULoLGameInstance>())
	{
		if (!GI->Nickname.IsEmpty())
		{
			ServerChangeName(GI->Nickname);
		}
	}
}

void ARiftPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwnedChamp = Cast<ALoLChampion>(InPawn);
	PRINTLOG_SH(TEXT("[OnPossess] OwnedChamp=%s"), *GetNameSafe(OwnedChamp));
}

void ARiftPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	ALoLChampion* Champion = Cast<ALoLChampion>(P);
	OwnedChamp = Champion;
	bCameraInitialized = false;
	if (!Champion) { return; }

	if (auto* GI = GetGameInstance<ULoLGameInstance>())
	{
		if (!GI->Nickname.IsEmpty())
		{
			ServerChangeName(GI->Nickname);
		}
	}

	PRINTLOG_SH(TEXT("[AcknowledgePossession] 챔피언=%s 위치=(%.0f,%.0f,%.0f)"),
	            *GetNameSafe(Champion),
	            Champion->GetActorLocation().X,
	            Champion->GetActorLocation().Y,
	            Champion->GetActorLocation().Z);

	if (ARiftHUD* HUD = GetHUD<ARiftHUD>())
	{
		HUD->InitHUD(OwnedChamp);
	}

	// CameraManager에 초기 위치 + 방향 설정
	TargetCameraLoc = Champion->GetActorLocation();
	if (ARiftPlayerCameraManager* Cam = GetRiftCameraManager())
	{
		Cam->CurrentCameraLoc = TargetCameraLoc;

		// 스폰 X좌표로 진영 판단 (팀 복제 타이밍과 무관하게 안정적)
		// Red spawn: X < 0  /  Blue spawn: X > 0
		/*const bool bRedSide = Champion->GetActorLocation().X < 0.f;
		Cam->SetTeamYaw(bRedSide);
		bIsRedTeam = bRedSide;*/
	}

	// 클라이언트에서 복제 완료 후 위치 교정 타이머
	GetWorldTimerManager().SetTimer(CameraInitTimer,
	                                [this]()
	                                {
		                                if (OwnedChamp && !OwnedChamp->GetActorLocation().IsNearlyZero())
		                                {
			                                TargetCameraLoc = OwnedChamp->GetActorLocation();
			                                if (ARiftPlayerCameraManager* Cam = GetRiftCameraManager())
			                                {
				                                Cam->CurrentCameraLoc = TargetCameraLoc;
			                                }
		                                }
	                                },
	                                0.3f,
	                                false);
}

void ARiftPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController()) { return; }

	// 커서 아래 적 유닛 감지 → 공격 커서로 전환
	{
		FHitResult CursorHit;
		GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

		bool bOverEnemy = false;
		if (CursorHit.bBlockingHit && OwnedChamp)
		{
			AActor* HitActor = CursorHit.GetActor();
			if (HitActor && HitActor != OwnedChamp &&
				HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()) &&
				!IDamageable::Execute_IsDead(HitActor))
			{
				// 챔피언에게 공격 커서 표시
				if (HitActor->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
				{
					ETeam HitTeam = ITargetable::Execute_GetTeam(HitActor);
					ETeam MyTeam = OwnedChamp->GetTeam_Implementation();
					EUnitType HitType = ITargetable::Execute_GetUnitType(HitActor);
					bOverEnemy = (HitTeam != MyTeam && HitTeam != ETeam::None
						&& HitType == EUnitType::Champion);
					// 타워/미니언도 포함하려면 아래 주석 해제
					// bOverEnemy = (HitTeam != MyTeam && HitTeam != ETeam::None);
				}
			}
		}

		CurrentMouseCursor = bOverEnemy ? EMouseCursor::Hand : EMouseCursor::Default;
	}

	ARiftPlayerCameraManager* Cam = GetRiftCameraManager();
	if (!Cam) { return; }

	// 챔피언 위치 복제 완료되면 카메라 한 번 스냅
	if (!bCameraInitialized && OwnedChamp)
	{
		FVector ChampLoc = OwnedChamp->GetActorLocation();
		if (!ChampLoc.IsNearlyZero())
		{
			TargetCameraLoc = ChampLoc;
			Cam->CurrentCameraLoc = ChampLoc;
			bCameraInitialized = true;
			PRINTLOG_SH(TEXT("[Camera] 스냅 완료 → %s"), *ChampLoc.ToString());
		}
	}

	UpdateIndicator();

	// 커서 페이싱 없음 — 이동 방향은 CMC가 처리, 스킬 방향은 Server_RequestSkill에서 처리

	if (bCameraLocked && OwnedChamp)
	{
		// 챔피언 잠금: 스페이스바
		TargetCameraLoc = OwnedChamp->GetActorLocation();
		Cam->CurrentCameraLoc = FMath::VInterpTo(Cam->CurrentCameraLoc, TargetCameraLoc, DeltaTime, CameraInterpSpeed);
	}
	else
	{
		// 카메라 초기화 완료 후에만 엣지스크롤 허용 (초기화 전 마우스 위치로 튀는 현상 방지)
		if (bCameraInitialized)
		{
			EdgeScrollWithMouse(DeltaTime);
		}

		if (bCameraBoundsEnabled)
		{
			TargetCameraLoc.X = FMath::Clamp(TargetCameraLoc.X, CameraBoundsMin.X, CameraBoundsMax.X);
			TargetCameraLoc.Y = FMath::Clamp(TargetCameraLoc.Y, CameraBoundsMin.Y, CameraBoundsMax.Y);
		}

		Cam->CurrentCameraLoc = FMath::VInterpTo(Cam->CurrentCameraLoc, TargetCameraLoc, DeltaTime, CameraInterpSpeed);
		// Cam->CurrentCameraLoc = TargetCameraLoc;
	}
}

// ----------------------------- Server -----------------------------------------
void ARiftPlayerController::Server_SelectTeam_Implementation(ETeam Team)
{
	if (APickWindowGameMode* GM = GetWorld()->GetAuthGameMode<APickWindowGameMode>())
	{
		GM->TrySwitchTeam(this, Team);
	}
}

void ARiftPlayerController::Server_SelectChampion_Implementation(FName ChampionID)
{
	if (auto* PS = GetPlayerState<ARiftPlayerState>())
	{
		PS->SetSelectedChampion(ChampionID);
	}
}

void ARiftPlayerController::Server_SetReady_Implementation()
{
	PRINTLOG_SH(TEXT("[Server_SetReady] 서버 도착. PS=%s"),
	            *GetNameSafe(GetPlayerState<ARiftPlayerState>()));

	if (auto* PS = GetPlayerState<ARiftPlayerState>())
	{
		PS->SetReady(true);
		PRINTLOG_SH(TEXT("[Server_SetReady] SetReady(true) 완료 → 이후 Travel 없음 (Host Start 대기)"));
	}
	// 절대 여기서 TryStartGame/Travel 호출 없음 — Host가 Start 눌렀을 때만
}

void ARiftPlayerController::Server_StartChampionSelect_Implementation()
{
	if (!HasAuthority()) { return; }

	GetWorld()->ServerTravel(TEXT("/Game/Maps/Lv_PickWindow?listen"));

	/*if (auto* GS = GetWorld()->GetGameState<ARiftGameState>())
	{
		GS->SetPhase(EGamePhase::ChampionSelect);
	}*/
}

void ARiftPlayerController::Server_StartGame_Implementation()
{
	if (!HasAuthority()) { return; }

	if (APickWindowGameMode* PGM = GetWorld()->GetAuthGameMode<APickWindowGameMode>())
	{
		PGM->TryStartGame(this);
	}
}


void ARiftPlayerController::EdgeScrollWithMouse(float DeltaTime)
{
	float mouseX, mouseY;
	if (!GetMousePosition(mouseX, mouseY)) { return; }

	int32 VPX, VPY;
	GetViewportSize(VPX, VPY);
	const FVector2D ViewportSize(VPX, VPY);
	// HUD 중간 영역(PlayableBottom ~ 화면 최하단 EdgeThreshold 직전)에서만 스크롤 차단
	// 화면 최하단 EdgeThreshold 구역은 허용 → 하단 edge scroll 정상 작동
	constexpr float BottomUIHeight = 180.f;
	const float PlayableBottom = ViewportSize.Y - BottomUIHeight;
	if (mouseY > PlayableBottom && mouseY < ViewportSize.Y - EdgeThreshold)
	{
		return;
	}

	FVector2D MoveInput = FVector2D::ZeroVector;

	if (mouseX <= EdgeThreshold)
	{
		MoveInput.X = -1.f;
	}

	else if (mouseX >= ViewportSize.X - EdgeThreshold)
	{
		MoveInput.X = 1.f;
	}

	if (mouseY <= EdgeThreshold)
	{
		MoveInput.Y = 1.f;
	}

	else if (mouseY >= ViewportSize.Y - EdgeThreshold)
	{
		MoveInput.Y = -1.f;
	}

	if (MoveInput.IsNearlyZero()) { return; }

	// 스폰 위치 기준으로 캐시된 팀 방향 사용
	const float Dir = bIsRedTeam ? -1.f : 1.f;
	const FVector Forward(0.f, Dir, 0.f);
	const FVector Right(-Dir, 0.f, 0.f);

	TargetCameraLoc += (Forward * MoveInput.Y + Right * MoveInput.X) * EdgeScrollSpeed * DeltaTime;
}

void ARiftPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* currContext : MappingContexts)
			{
				Subsystem->AddMappingContext(currContext, 0);
			}
		}
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC) { return; }

	EIC->BindAction(IA_LockCam, ETriggerEvent::Started, this, &ARiftPlayerController::OnCameraLockToggled);
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Triggered, this, &ARiftPlayerController::OnCameraFocusHeld);
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Completed, this, &ARiftPlayerController::OnCameraFocusReleased);
	EIC->BindAction(IA_Move, ETriggerEvent::Started, this, &ARiftPlayerController::OnMove);
	EIC->BindAction(IA_Exit, ETriggerEvent::Started, this, &ARiftPlayerController::OnExit);

	EIC->BindAction(IA_SkillQ, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillQPressed);
	EIC->BindAction(IA_SkillQ, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillQReleased);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillWPressed);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillWReleased);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillEPressed);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillEReleased);
	EIC->BindAction(IA_SkillR, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillRPressed);
	EIC->BindAction(IA_SkillR, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillRReleased);
	EIC->BindAction(IA_Attack_A, ETriggerEvent::Started, this, &ARiftPlayerController::OnAPressed);
	EIC->BindAction(IA_Attack_A, ETriggerEvent::Completed, this, &ARiftPlayerController::OnAReleased);
	EIC->BindAction(IA_LeftClick, ETriggerEvent::Started, this, &ARiftPlayerController::OnLeftClick);
	EIC->BindAction(IA_Shop, ETriggerEvent::Started, this, &ARiftPlayerController::OnToggleShop);

	EIC->BindAction(IA_LevelUp, ETriggerEvent::Started, this, &ARiftPlayerController::Server_AddXP);
	EIC->BindAction(IA_Spell_D, ETriggerEvent::Started, this, &ARiftPlayerController::OnSpellD);
	EIC->BindAction(IA_Spell_F, ETriggerEvent::Started, this, &ARiftPlayerController::OnSpellF);
}

void ARiftPlayerController::OnCameraFocusHeld()
{
	if (OwnedChamp)
	{
		TargetCameraLoc = OwnedChamp->GetActorLocation();
	}
}

void ARiftPlayerController::OnCameraFocusReleased()
{
	// TargetCameraLoc은 챔피언 시점에서 멈춤
}

void ARiftPlayerController::OnMove()
{
	if (!OwnedChamp)
	{
		PRINTLOG_SH(TEXT("[OnMove] OwnedChamp=null — 이동 불가"));
		return;
	}

	/*// MaxWalkSpeed 0이면 로그
	if (UCharacterMovementComponent* MC = OwnedChamp->GetCharacterMovement())
	{
		if (MC->MaxWalkSpeed < 1.f)
		{
			PRINTLOG_SH(TEXT("[OnMove] MaxWalkSpeed=0 — R 장전 잠금 상태"));
			return;
		}
	}*/

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	PRINTLOG_SH(TEXT("[OnMove] Hit=%d Actor=%s Loc=%s"),
		HitResult.bBlockingHit,
		*GetNameSafe(HitResult.GetActor()),
		*HitResult.ImpactPoint.ToString());

	if (!HitResult.bBlockingHit) { return; }

	// 우클릭: 커서가 직접 적 위에 올라간 경우만 공격 (영역 탐색 제거)
	// A클릭 영역 공격은 TryBasicAttackAtCursor에서 별도 처리
	AActor* AttackTarget = nullptr;
	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor != OwnedChamp &&
		HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()) &&
		!IDamageable::Execute_IsDead(HitActor) &&
		HitActor->GetClass()->ImplementsInterface(UTargetable::StaticClass()))
	{
		ETeam ActorTeam = ITargetable::Execute_GetTeam(HitActor);
		if (ActorTeam != OwnedChamp->GetTeam_Implementation() && ActorTeam != ETeam::None)
		{
			AttackTarget = HitActor;
		}
	}

	if (AttackTarget)
	{
		Server_RequestBasicAttack(AttackTarget);
		return;
	}

	// 서버 권위 이동 (서버에서 StopAttackLoop + SetMoveTarget 처리)
	Server_MoveToLocation(HitResult.ImpactPoint);

	// 로컬 예측 이동
	OwnedChamp->SetMoveTarget(HitResult.ImpactPoint);
}

void ARiftPlayerController::OnCameraLockToggled()
{
	bCameraLocked = !bCameraLocked;
}

void ARiftPlayerController::OnAPressed()
{
	bAKeyPressed = true;
}

void ARiftPlayerController::OnAReleased()
{
	bAKeyPressed = false;
}

void ARiftPlayerController::OnLeftClick()
{
	if (bAKeyPressed)
	{
		TryBasicAttackAtCursor();
	}
}

void ARiftPlayerController::OnToggleShop()
{
	if (ARiftHUD* HUD = GetHUD<ARiftHUD>())
	{
		HUD->ToggleShop();
	}
}

void ARiftPlayerController::OnExit()
{
	const FString CurrentMap = UGameplayStatics::GetCurrentLevelName(this, true);
	PRINTLOG_SH(TEXT("[OnExit] 현재 맵: %s"), *CurrentMap);

	if (!CurrentMap.Contains(TEXT("SummonerRift"))) { return; }

	ARiftHUD* HUD = GetHUD<ARiftHUD>();
	PRINTLOG_SH(TEXT("[OnExit] HUD: %s"), *GetNameSafe(HUD));

	if (HUD)
	{
		HUD->ToggleExitPopup();
	}
}

void ARiftPlayerController::TryBasicAttackAtCursor()
{
	if (!OwnedChamp) { return; }

	// 공격 대상 클릭했는지 확인
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	AActor* Target = nullptr;

	if (HitResult.bBlockingHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor != OwnedChamp &&
			HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()) &&
			!IDamageable::Execute_IsDead(HitActor))
		{
			Target = HitActor;
		}
	}

	// 클릭한 타겟 없으면 커서 주변 가장 가까운 타겟 탐색 (챔피언, 미니언, 포탑 포함)
	if (!Target)
	{
		const FVector CursorLoc = HitResult.bBlockingHit ? HitResult.ImpactPoint : OwnedChamp->GetActorLocation();
		float NearestDist = FLT_MAX;

		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UDamageable::StaticClass(), AllActors);

		for (AActor* Actor : AllActors)
		{
			if (Actor == OwnedChamp) { continue; }
			if (IDamageable::Execute_IsDead(Actor)) { continue; }

			float Dist = FVector::Dist(Actor->GetActorLocation(), CursorLoc);
			if (Dist < NearestDist)
			{
				NearestDist = Dist;
				Target = Actor;
			}
		}
	}

	if (Target)
	{
		Server_RequestBasicAttack(Target);
		PRINTLOG_SH(TEXT("A+LMB 평타 -> %s"), *GetNameSafe(Target));
	}
}

void ARiftPlayerController::OnSkillQPressed() { ShowSkillIndicator(ESkillSlot::Q); }
void ARiftPlayerController::OnSkillQReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillWPressed() { ShowSkillIndicator(ESkillSlot::W); }
void ARiftPlayerController::OnSkillWReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillEPressed() { ShowSkillIndicator(ESkillSlot::E); }
void ARiftPlayerController::OnSkillEReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillRPressed() { ShowSkillIndicator(ESkillSlot::R); }
void ARiftPlayerController::OnSkillRReleased() { FirePendingSkill(); }

void ARiftPlayerController::ShowSkillIndicator(ESkillSlot Slot)
{
	if (!OwnedChamp) { return; }

	// 랭크 0이면 인디케이터 표시 안 함
	if (OwnedChamp->SkillComp && OwnedChamp->SkillComp->GetRank(Slot) == 0) { return; }

	HideSkillIndicator();
	PendingSkillSlot = static_cast<int32>(Slot);

	// E는 원형, Q/W는 선형
	TSubclassOf<AActor> IndicatorClass = (Slot == ESkillSlot::E)
		                                     ? CircleIndicatorClass
		                                     : LineIndicatorClass;

	if (!IndicatorClass) { return; }

	FVector SpawnLoc = OwnedChamp->GetActorLocation();
	CurrentIndicator = GetWorld()->SpawnActor<AActor>(IndicatorClass, SpawnLoc, FRotator::ZeroRotator);
	if (CurrentIndicator)
	{
		CurrentIndicator->AttachToActor(OwnedChamp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void ARiftPlayerController::HideSkillIndicator()
{
	if (CurrentIndicator)
	{
		CurrentIndicator->Destroy();
		CurrentIndicator = nullptr;
	}
	PendingSkillSlot = -1;
}

void ARiftPlayerController::UpdateIndicator()
{
	if (!CurrentIndicator || !OwnedChamp || PendingSkillSlot < 0) { return; }

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if (!HitResult.bBlockingHit) { return; }

	FVector CursorLoc = HitResult.ImpactPoint;
	FVector ChampLoc = OwnedChamp->GetActorLocation();

	FVector Dir = (CursorLoc - ChampLoc).GetSafeNormal2D();
	if (Dir.IsNearlyZero()) { return; }

	// Attach되어 있으므로 위치는 자동, 방향만 업데이트
	CurrentIndicator->SetActorRotation(Dir.ToOrientationRotator());
}

void ARiftPlayerController::FirePendingSkill()
{
	if (PendingSkillSlot < 0) { return; }

	// 클라이언트는 Ranks가 복제 안 되므로 랭크 체크 스킵 — 서버가 RequestActivateSkill에서 검증

	ESkillSlot Slot = static_cast<ESkillSlot>(PendingSkillSlot);
	HideSkillIndicator();
	RequestSkill(Slot);
}

void ARiftPlayerController::RequestSkill(ESkillSlot Slot)
{
	PRINTLOG_SH(TEXT("[RequestSkill] OwnedChamp=%s IsLocal=%d"),
		*GetNameSafe(OwnedChamp), IsLocalController());
	if (!OwnedChamp || !OwnedChamp->SkillComp) { return; }

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	FVector TargetLoc = HitResult.bBlockingHit
		                    ? HitResult.ImpactPoint
		                    : OwnedChamp->GetActorLocation() + OwnedChamp->GetActorForwardVector() * 1000.f;

	Server_RequestSkill(Slot, TargetLoc);
}

void ARiftPlayerController::Server_RequestSkill_Implementation(ESkillSlot Slot, FVector TargetLoc)
{
	PRINTLOG_SH(TEXT("[Skill RPC] Slot=%d OwnedChamp=%s SkillComp=%s Executor=%s"),
		(int32)Slot,
		*GetNameSafe(OwnedChamp),
		OwnedChamp ? *GetNameSafe(OwnedChamp->SkillComp) : TEXT("NULL"),
		(OwnedChamp && OwnedChamp->SkillExecutor) ? TEXT("OK") : TEXT("NULL"));

	if (!OwnedChamp || !OwnedChamp->SkillComp) { return; }

	// 스킬 발사 전 마우스 방향으로 한 번만 회전 (Tick 커서 페이싱 대신)
	FVector Dir = (TargetLoc - OwnedChamp->GetActorLocation());
	Dir.Z = 0.f;
	if (!Dir.IsNearlyZero())
	{
		OwnedChamp->SetActorRotation(Dir.GetSafeNormal().Rotation());
	}

	OwnedChamp->SkillComp->RequestActivateSkill(Slot, TargetLoc);
}

void ARiftPlayerController::Server_RequestBasicAttack_Implementation(AActor* Target)
{
	if (!OwnedChamp || !Target) { return; }
	OwnedChamp->StartAttackLoop(Target);
}

void ARiftPlayerController::Server_MoveToLocation_Implementation(FVector Loc)
{
	if (!OwnedChamp) { return; }

	OwnedChamp->StopAttackLoop();
	OwnedChamp->SetMoveTarget(Loc);
}

void ARiftPlayerController::Server_AssignSkillPoint_Implementation(ESkillSlot Slot)
{
	if (!OwnedChamp || !OwnedChamp->SkillComp || !OwnedChamp->StatComp) { return; }

	// R은 6/11/16 레벨에서만 찍을 수 있음
	if (Slot == ESkillSlot::R)
	{
		const int32 Level = OwnedChamp->StatComp->GetLevel();
		const int32 CurRank = OwnedChamp->SkillComp->GetRank(ESkillSlot::R);
		constexpr int32 RLevelReq[] = {6, 11, 16};

		if (CurRank >= 3 || Level < RLevelReq[CurRank])
		{
			return;
		}
	}

	if (OwnedChamp->SkillComp->AssignSkillPoint(Slot))
	{
		Client_OnSkillAssigned(Slot);
	}
}

void ARiftPlayerController::Client_OnSkillAssigned_Implementation(ESkillSlot Slot)
{
	// Listen Server 호스트는 서버에서 이미 처리했으므로 스킵
	// 순수 클라이언트만 랭크 업데이트 + UI 갱신
	if (!HasAuthority() && OwnedChamp && OwnedChamp->SkillComp)
	{
		OwnedChamp->SkillComp->ApplySkillPointClient(Slot);
	}
}

void ARiftPlayerController::Server_AddXP_Implementation()
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	const int32 PrevLevel = PS->GetChampionLevel();
	PS->AddXP(200.f);

	// 레벨업: StatComp 동기화
	const int32 NewLevel = PS->GetChampionLevel();
	if (NewLevel > PrevLevel && OwnedChamp && OwnedChamp->StatComp)
	{
		OwnedChamp->StatComp->SetLevel(NewLevel);
		PRINTLOG_SH(TEXT("[Debug] LevelUp → Lv.%d"), NewLevel);
	}
	else
	{
		PRINTLOG_SH(TEXT("[Debug] XP +50 → %.0f / %.0f"),
		            PS->GetXP(),
		            NewLevel < 18 ? 280.f : 0.f);
	}
}

void ARiftPlayerController::Server_SelectSummonerSpells_Implementation(ESummonerSpell Spell1, ESummonerSpell Spell2)
{
	// TODO: 픽창 스펠 선택 UI 구현 후 주석 해제
	// ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	// if (!PS) { return; }
	// PS->SetSummonerSpells(Spell1, Spell2);
}

// ---------------스펠---------------
void ARiftPlayerController::OnSpellD()
{
	if (!OwnedChamp) { return; }
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	FVector TargetLoc = Hit.bBlockingHit ? Hit.ImpactPoint : OwnedChamp->GetActorLocation();
	Server_CastSummonerSpell(0, TargetLoc);
}

void ARiftPlayerController::OnSpellF()
{
	if (!OwnedChamp) { return; }
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);
	FVector TargetLoc = Hit.bBlockingHit ? Hit.ImpactPoint : OwnedChamp->GetActorLocation();
	Server_CastSummonerSpell(1, TargetLoc);
}

void ARiftPlayerController::Server_CastSummonerSpell_Implementation(int32 SlotIndex, FVector TargetLoc)
{
	if (!OwnedChamp) { return; }

	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	ESummonerSpell Spell = (SlotIndex == 0) ? PS->GetSummonerSpell1() : PS->GetSummonerSpell2();
	if (Spell == ESummonerSpell::None) { return; }

	// 스펠별 쿨타임 (FindRow 불일치 우회용 하드코딩)
	auto GetSpellCooldown = [](ESummonerSpell S) -> float
	{
		switch (S)
		{
		case ESummonerSpell::Flash: return 300.f;
		case ESummonerSpell::Heal: return 240.f;
		case ESummonerSpell::Ignite: return 180.f;
		case ESummonerSpell::Ghost: return 210.f;
		case ESummonerSpell::Exhaust: return 210.f;
		case ESummonerSpell::Cleanse: return 210.f;
		default: return 300.f;
		}
	};

	const FName SpellRowName = FName(*UEnum::GetValueAsString(Spell).Replace(TEXT("ESummonerSpell::"), TEXT("")));

	UCooldownComponent* CD = OwnedChamp->CooldownComp;
	const FName CoolTag = FName(*FString::Printf(TEXT("SummonerSpell.%d"), SlotIndex));
	if (CD && CD->IsOnCooldown(CoolTag)) { return; }

	// Step 1: SpellBase 조회
	float Cooldown = GetSpellCooldown(Spell); 
	float Range = 400.f;
	float BaseValue = 0.f;
	float ValueGrowth = 0.f;
	FString TargetLogicID;
	FString EffectTag;

	if (SpellBaseTable)
	{
		if (const FSpellBaseRow* Row = SpellBaseTable->FindRow<FSpellBaseRow>(SpellRowName, TEXT("")))
		{
			Range = Row->Range;
			BaseValue = Row->BaseValue;
			ValueGrowth = Row->ValueGrowth;
			TargetLogicID = Row->TargetLogic_ID;
			EffectTag = Row->EffectTag;
		}
	}

	// 레벨 보정 (PlayerState ChampionLevel 기준)
	const int32 Level = PS->GetChampionLevel();
	const float FinalValue = BaseValue + ValueGrowth * FMath::Max(Level - 1, 0);

	// Step 2: SpellTargeting 조회 (TargetLogicID 키 사용)
	float SearchRadius = Range;
	bool bAffectsAlly = false;
	int32 MaxTarget = 1;

	if (SpellTargetingTable && !TargetLogicID.IsEmpty())
	{
		if (const FSpellTargetingRow* Row = SpellTargetingTable->FindRow<FSpellTargetingRow>(
			FName(*TargetLogicID),
			TEXT("")))
		{
			bAffectsAlly = Row->AffectsAlly;
			MaxTarget = Row->MaxTarget;
			SearchRadius = (Row->SearchRadius > 0) ? (float)Row->SearchRadius : Range;
		}
	}

	// Step 3: SpellSecondaryEffect 조회 (EffectTag 키 사용)
	float SecondaryValue = 0.f;

	if (SpellSecondaryEffectTable && !EffectTag.IsEmpty())
	{
		if (const FSpellSecondaryEffectRow* Row = SpellSecondaryEffectTable->FindRow<FSpellSecondaryEffectRow>(
			FName(*EffectTag),
			TEXT("")))
		{
			SecondaryValue = Row->SecondaryValue;
		}
	}

	switch (Spell)
	{
	case ESummonerSpell::Flash:
		{
			// Range = 최대 블링크 거리
			FVector Dir = (TargetLoc - OwnedChamp->GetActorLocation()).GetSafeNormal2D();
			float Dist = FVector::Dist2D(OwnedChamp->GetActorLocation(), TargetLoc);
			FVector Dest = OwnedChamp->GetActorLocation() + Dir * FMath::Min(Dist, Range);
			OwnedChamp->TeleportTo(Dest, OwnedChamp->GetActorRotation());
			break;
		}
	case ESummonerSpell::Heal:
		{
			// 자신 회복 (레벨 보정 적용)
			if (OwnedChamp->StatComp)
			{
				OwnedChamp->StatComp->ApplyHealthChange(FinalValue);
			}
			// 주변 아군 챔피언 회복 (SecondaryValue = 아군 회복 비율)
			if (bAffectsAlly)
			{
				const float AllyHeal = (SecondaryValue > 0.f) ? FinalValue * SecondaryValue : FinalValue * 0.3f;
				int32 HealedAllies = 0;

				TArray<AActor*> Actors;
				UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UTargetable::StaticClass(), Actors);
				// 가장 HP가 낮은 아군 우선 (Logic_HealPriority)
				Actors.Sort([](const AActor& A, const AActor& B)
				{
					UStatComponent* SA = A.FindComponentByClass<UStatComponent>();
					UStatComponent* SB = B.FindComponentByClass<UStatComponent>();
					float RatioA = SA ? SA->GetCurrentHP() / FMath::Max(SA->GetMaxHP(), 1.f) : 1.f;
					float RatioB = SB ? SB->GetCurrentHP() / FMath::Max(SB->GetMaxHP(), 1.f) : 1.f;
					return RatioA < RatioB;
				});

				for (AActor* Actor : Actors)
				{
					if (HealedAllies >= MaxTarget - 1) { break; } // -1: 자신 제외
					if (Actor == OwnedChamp) { continue; }
					if (ITargetable::Execute_GetTeam(Actor) != OwnedChamp->GetTeam_Implementation()) { continue; }
					if (ITargetable::Execute_GetUnitType(Actor) != EUnitType::Champion) { continue; }
					if (FVector::Dist2D(Actor->GetActorLocation(), OwnedChamp->GetActorLocation()) > SearchRadius)
					{
						continue;
					}
					if (UStatComponent* SC = Actor->FindComponentByClass<UStatComponent>())
					{
						SC->ApplyHealthChange(AllyHeal);
						HealedAllies++;
					}
				}
			}
			break;
		}
	default: break;
	}

	if (CD)
	{
		CD->StartCooldown(CoolTag, Cooldown);
	}

	// 클라이언트 UI에 쿨타임 알림
	Client_OnSpellCast(SlotIndex, Cooldown);
}

void ARiftPlayerController::Client_OnSpellCast_Implementation(int32 SlotIndex, float Cooldown)
{
	ARiftHUD* HUD = GetHUD<ARiftHUD>();
	if (!HUD) { return; }

	USkillBarWidget* Bar = HUD->GetMainHUDWidget() ? HUD->GetMainHUDWidget()->GetSkillBar() : nullptr;
	if (!Bar) { return; }

	USpellSlotWidget* Slot = (SlotIndex == 0) ? Bar->GetSpellD() : Bar->GetSpellF();
	if (Slot)
	{
		Slot->TriggerCooldown(Cooldown);
	}
}

void ARiftPlayerController::Server_SelectLane_Implementation(ELane Lane)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	PS->SetLane(Lane);
}
