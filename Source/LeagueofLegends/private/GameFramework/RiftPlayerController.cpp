#include "GameFramework/RiftPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "LeagueofLegends.h"
#include "AStar/AStarGridManager.h"
#include "Engine/LocalPlayer.h"
#include "Characters/LoLChampion.h"
#include "Characters/LoLCharacterBase.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/PickWindowGameMode.h"
#include "GameFramework/RiftPlayerState.h"
#include "GameFramework/RiftHUD.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/CombatComponent.h"
#include "Components/StatComponent.h"
#include "GameFramework/LoLGameInstance.h"
#include "GameFramework/RiftGameState.h"
#include "Interfaces/Damageable.h"
#include "Kismet/GameplayStatics.h"

ARiftPlayerController::ARiftPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
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

	// 닉네임 재전송 — Seamless Travel 후 PlayerState가 새로 생성될 수 있으므로 여기서 확실히 보냄
	if (auto* GI = GetGameInstance<ULoLGameInstance>())
	{
		if (!GI->Nickname.IsEmpty())
		{
			ServerChangeName(GI->Nickname);
		}
	}

	PRINTLOG_SH(TEXT("[AcknowledgePossession] 챔피언=%s 위치=(%.0f,%.0f,%.0f) CameraClass=%s"),
		*GetNameSafe(Champion),
		Champion->GetActorLocation().X, Champion->GetActorLocation().Y, Champion->GetActorLocation().Z,
		*GetNameSafe(CameraActorClass));

	if (ARiftHUD* HUD = GetHUD<ARiftHUD>())
	{
		HUD->InitHUD(OwnedChamp);
	}

	FVector CameraStartLoc = Champion->GetActorLocation();

	if (CameraActorClass)
	{
		CameraActor = GetWorld()->SpawnActor<ALoLCameraActor>(CameraActorClass,
		                                                      FTransform(FRotator::ZeroRotator, CameraStartLoc));
	}

	if (!CameraActor)
	{
		PRINTLOG_SH(TEXT("[AcknowledgePossession] CameraActor 스폰 실패. CameraActorClass=%s"),
			*GetNameSafe(CameraActorClass));
		return;
	}

	// UE의 자동 ViewTarget 관리를 끔 — 폰 소유 시 ViewTarget이 폰으로 덮어씌워지는 것을 방지
	bAutoManageActiveCameraTarget = false;

	TargetCameraLoc = CameraStartLoc;
	SetViewTarget(CameraActor);

	// 클라이언트에서 첫 위치 복제 전에 스폰될 수 있으므로 교정 (0.3s, 1.0s 두 번)
	GetWorldTimerManager().SetTimer(CameraInitTimer, [this]()
	{
		if (CameraActor && OwnedChamp)
		{
			CameraActor->SetActorLocation(OwnedChamp->GetActorLocation());
			TargetCameraLoc = OwnedChamp->GetActorLocation();
		}
	}, 0.3f, false);

	FTimerHandle CameraInitTimer2;
	GetWorldTimerManager().SetTimer(CameraInitTimer2, [this]()
	{
		if (CameraActor && OwnedChamp)
		{
			CameraActor->SetActorLocation(OwnedChamp->GetActorLocation());
			TargetCameraLoc = OwnedChamp->GetActorLocation();
		}
	}, 1.0f, false);
}

void ARiftPlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	if (CameraActor)
	{
		SetViewTarget(CameraActor);
		return;
	}

	Super::AutoManageActiveCameraTarget(SuggestedTarget);
}

void ARiftPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController() || !CameraActor) { return; }

	// 클라이언트: 챔피언 위치가 복제 완료되면 카메라 한 번 스냅
	if (!bCameraInitialized && OwnedChamp)
	{
		FVector ChampLoc = OwnedChamp->GetActorLocation();

		PRINTLOG_SH(TEXT("[Camera] Local=%d OwnedChamp=%s ChampLoc=%s CameraLoc=%s"),
			IsLocalController() ? 1 : 0,
			*GetNameSafe(OwnedChamp),
			*ChampLoc.ToString(),
			*CameraActor->GetActorLocation().ToString());

		if (!ChampLoc.IsNearlyZero())
		{
			CameraActor->SetActorLocation(ChampLoc);
			TargetCameraLoc = ChampLoc;
			bCameraInitialized = true;
			PRINTLOG_SH(TEXT("[Camera] 스냅 완료 → %s"), *ChampLoc.ToString());
		}
	}

	UpdateIndicator();

	if (bCameraLocked && OwnedChamp)
	{
		TargetCameraLoc = OwnedChamp->GetActorLocation();
		/*FVector NewLoc = FMath::VInterpTo(CameraActor->GetActorLocation(), TargetCameraLoc, DeltaTime, CameraInterpSpeed);
		CameraActor->SetActorLocation(NewLoc);*/
	}
	else
	{
		EdgeScrollWithMouse(DeltaTime);
	}

	// GEngine->AddOnScreenDebugMessage(0, 0.f, FColor::Yellow, FString::Printf(TEXT("******CamXY: %.0f, %.0f"), TargetCameraLoc.X, TargetCameraLoc.Y));

	if (bCameraBoundsEnabled)
	{
		TargetCameraLoc.X = FMath::Clamp(TargetCameraLoc.X, CameraBoundsMin.X, CameraBoundsMax.X);
		TargetCameraLoc.Y = FMath::Clamp(TargetCameraLoc.Y, CameraBoundsMin.Y, CameraBoundsMax.Y);
	}

	FVector NewLoc = FMath::VInterpTo(CameraActor->GetActorLocation(), TargetCameraLoc, DeltaTime, CameraInterpSpeed);
	CameraActor->SetActorLocation(NewLoc);
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
	if (!CameraActor) { return; }

	float mouseX, mouseY;

	if (!GetMousePosition(mouseX, mouseY)) { return; }

	if (GEngine && GEngine->GameViewport)
	{
		FVector2D viewportSize;
		GEngine->GameViewport->GetViewportSize(viewportSize);
		FVector2D moveInput = FVector2D::ZeroVector;

		if (mouseX < EdgeThreshold)
		{
			// PRINTLOG_SH(TEXT("Left***"));
			moveInput.X = -1.f;
		}
		if (mouseX > viewportSize.X - EdgeThreshold)
		{
			// PRINTLOG_SH(TEXT("Right***"));
			moveInput.X = 1.f;
		}
		if (mouseY < EdgeThreshold)
		{
			// PRINTLOG_SH(TEXT("Top***"));
			moveInput.Y = 1.f;
		}
		if (mouseY > viewportSize.Y - EdgeThreshold)
		{
			// PRINTLOG_SH(TEXT("Bottom***"));
			moveInput.Y = -1.f;
		}

		if (moveInput.IsNearlyZero()) { return; }

		FVector Forward = CameraActor->GetViewForwardXY();
		FVector Right = CameraActor->GetViewRightXY();

		TargetCameraLoc += (Forward * moveInput.Y + Right * moveInput.X) * EdgeScrollSpeed * DeltaTime;
	}
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

	EIC->BindAction(IA_SkillQ, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillQPressed);
	EIC->BindAction(IA_SkillQ, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillQReleased);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillWPressed);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillWReleased);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillEPressed);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillEReleased);
	EIC->BindAction(IA_SkillR, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillRPressed);
	EIC->BindAction(IA_SkillR, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillRReleased);
	EIC->BindAction(IA_Attack_A, ETriggerEvent::Started, this, &ARiftPlayerController::OnAPressed);
	EIC->BindAction(IA_Attack_A, ETriggerEvent::Completed, this, &ARiftPlayerController::OnAReleased);
	EIC->BindAction(IA_LeftClick, ETriggerEvent::Started, this, &ARiftPlayerController::OnLeftClick);
	EIC->BindAction(IA_Shop, ETriggerEvent::Started, this, &ARiftPlayerController::OnToggleShop);

	EIC->BindAction(IA_LevelUp, ETriggerEvent::Started, this, &ARiftPlayerController::Server_AddXP);
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
	if (!OwnedChamp) { return; }

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	if (!HitResult.bBlockingHit) { return; }

	// 커서가 적 위에 있으면 이동 대신 공격
	AActor* HitActor = HitResult.GetActor();
	if (HitActor && HitActor != OwnedChamp &&
		HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()) &&
		!IDamageable::Execute_IsDead(HitActor))
	{
		Server_RequestBasicAttack(HitActor);
		return;
	}

	Server_MoveToLocation(HitResult.ImpactPoint);
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

void ARiftPlayerController::OnSkillQPressed()  { ShowSkillIndicator(ESkillSlot::Q); }
void ARiftPlayerController::OnSkillQReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillWPressed()  { ShowSkillIndicator(ESkillSlot::W); }
void ARiftPlayerController::OnSkillWReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillEPressed()  { ShowSkillIndicator(ESkillSlot::E); }
void ARiftPlayerController::OnSkillEReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillRPressed()  { ShowSkillIndicator(ESkillSlot::R); }
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
		? CircleIndicatorClass : LineIndicatorClass;

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
	FVector ChampLoc  = OwnedChamp->GetActorLocation();

	FVector Dir = (CursorLoc - ChampLoc).GetSafeNormal2D();
	if (Dir.IsNearlyZero()) { return; }

	// Attach되어 있으므로 위치는 자동, 방향만 업데이트
	CurrentIndicator->SetActorRotation(Dir.ToOrientationRotator());
}

void ARiftPlayerController::FirePendingSkill()
{
	if (PendingSkillSlot < 0) { return; }

	// 랭크 0이면 발사 차단
	if (OwnedChamp && OwnedChamp->SkillComp)
	{
		ESkillSlot Slot = static_cast<ESkillSlot>(PendingSkillSlot);
		if (OwnedChamp->SkillComp->GetRank(Slot) == 0)
		{
			HideSkillIndicator();
			return;
		}
	}

	ESkillSlot Slot = static_cast<ESkillSlot>(PendingSkillSlot);
	HideSkillIndicator();
	RequestSkill(Slot);
}

void ARiftPlayerController::RequestSkill(ESkillSlot Slot)
{
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
	if (!OwnedChamp || !OwnedChamp->SkillComp) { return; }
	OwnedChamp->SkillComp->RequestActivateSkill(Slot, TargetLoc);
}

void ARiftPlayerController::Server_RequestBasicAttack_Implementation(AActor* Target)
{
	if (!OwnedChamp || !Target) { return; }
	OwnedChamp->StartAttackLoop(Target);
}

void ARiftPlayerController::Server_MoveToLocation_Implementation(FVector Loc)
{
	// PRINTLOG_SH(TEXT("[Server_Move] OwnedChamp=%s Loc=%s"), *GetNameSafe(OwnedChamp), *Loc.ToString());
	if (OwnedChamp)
	{
		OwnedChamp->StopAttackLoop();
	}
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Loc);
}

void ARiftPlayerController::Server_AssignSkillPoint_Implementation(ESkillSlot Slot)
{
	if (!OwnedChamp || !OwnedChamp->SkillComp) { return; }
	if (OwnedChamp->SkillComp->AssignSkillPoint(Slot))
	{
		Client_OnSkillAssigned(Slot); // 본인 클라이언트에만 전송
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
		PRINTLOG_SH(TEXT("[Debug] XP +50 → %.0f / %.0f"), PS->GetXP(),
			NewLevel < 18 ? 280.f : 0.f); 
	}
}

void ARiftPlayerController::Server_SelectSummonerSpells_Implementation(ESummonerSpell Spell1, ESummonerSpell Spell2)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	PS->SetSummonerSpells(Spell1, Spell2);
}

void ARiftPlayerController::Server_SelectLane_Implementation(ELane Lane)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) { return; }

	PS->SetLane(Lane);
}
