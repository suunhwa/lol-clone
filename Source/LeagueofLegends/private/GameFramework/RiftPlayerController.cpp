#include "GameFramework/RiftPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "LeagueofLegends.h"
#include "AStar/AStarGridManager.h"
#include "Engine/LocalPlayer.h"
#include "Characters/LoLChampion.h"
#include "Characters/LoLCharacterBase.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftPlayerState.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/CombatComponent.h"
#include "Components/StatComponent.h"
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

	if (!IsLocalController()) return;

	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// A* 로직 완성되면
	// GridManager = Cast<AAStarGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAStarGridManager::StaticClass()));
}

void ARiftPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	ALoLChampion* Champion = Cast<ALoLChampion>(P);
	OwnedChamp = Champion;
	if (!Champion) return;

	FVector CameraStartLoc = Champion->GetActorLocation();

	if (CameraActorClass)
	{
		CameraActor = GetWorld()->SpawnActor<ALoLCameraActor>(CameraActorClass,
		                                                      FTransform(FRotator::ZeroRotator, CameraStartLoc));
	}

	TargetCameraLoc = CameraStartLoc;
	SetViewTarget(CameraActor);
}

void ARiftPlayerController::AutoManageActiveCameraTarget(AActor* SuggestedTarget)
{
	if (CameraActor)
	{
		// SetViewTarget(CameraActor);
		return;
	}

	Super::AutoManageActiveCameraTarget(SuggestedTarget);
}

void ARiftPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController() || !CameraActor) { return; }

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

	EIC->BindAction(IA_SkillQ, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillQPressed);
	EIC->BindAction(IA_SkillQ, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillQReleased);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillWPressed);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillWReleased);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillEPressed);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Completed, this, &ARiftPlayerController::OnSkillEReleased);
	EIC->BindAction(IA_SkillR, ETriggerEvent::Started,   this, &ARiftPlayerController::OnSkillR);
	EIC->BindAction(IA_Attack_A, ETriggerEvent::Started, this, &ARiftPlayerController::OnAPressed);
	EIC->BindAction(IA_Attack_A, ETriggerEvent::Completed, this, &ARiftPlayerController::OnAReleased);
	EIC->BindAction(IA_LeftClick, ETriggerEvent::Started, this, &ARiftPlayerController::OnLeftClick);
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

	/*PRINTLOG_SH(TEXT("OnMove — bHit:%d Loc:%s Actor:%s"),
		HitResult.bBlockingHit,
		*HitResult.ImpactPoint.ToString(),
		*GetNameSafe(HitResult.GetActor()));*/

	if (!HitResult.bBlockingHit) { return; }

	// 커서가 적 위에 있으면 이동 대신 공격
	ALoLCharacterBase* HitChar = Cast<ALoLCharacterBase>(HitResult.GetActor());
	if (HitChar && HitChar != OwnedChamp)
	{
		UStatComponent* TargetStat = HitChar->FindComponentByClass<UStatComponent>();
		if (TargetStat && !TargetStat->IsDead())
		{
			Server_RequestBasicAttack(HitChar);
			return;
		}
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

void ARiftPlayerController::TryBasicAttackAtCursor()
{
	if (!OwnedChamp) { return; }

	// 공격 대상 클릭했는지 확인
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	AActor* Target = nullptr;

	if (HitResult.bBlockingHit && HitResult.GetActor())
	{
		UStatComponent* Stat = HitResult.GetActor()->FindComponentByClass<UStatComponent>();
		if (Stat && !Stat->IsDead() && HitResult.GetActor() != OwnedChamp)
		{
			Target = HitResult.GetActor();
		}
	}

	// 클릭한 타겟 없으면 커서 주변 가장 가까운 타겟 탐색
	if (!Target)
	{
		const FVector CursorLoc = HitResult.bBlockingHit ? HitResult.ImpactPoint : OwnedChamp->GetActorLocation();

		float NearestDist = FLT_MAX;

		TArray<AActor*> allActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALoLCharacterBase::StaticClass(), allActors);

		for (AActor* Actor : allActors)
		{
			if (Actor == OwnedChamp) { continue; }

			UStatComponent* Stat = Actor->FindComponentByClass<UStatComponent>();
			if (!Stat || Stat->IsDead()) { continue; }

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
		ALoLCharacterBase* TargetChar = Cast<ALoLCharacterBase>(Target);
		if (TargetChar)
		{
			Server_RequestBasicAttack(TargetChar);
			PRINTLOG_SH(TEXT("A+LMB 평타 -> %s"), *GetNameSafe(Target));
		}
	}
}

void ARiftPlayerController::OnSkillQPressed()  { ShowSkillIndicator(ESkillSlot::Q); }
void ARiftPlayerController::OnSkillQReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillWPressed()  { ShowSkillIndicator(ESkillSlot::W); }
void ARiftPlayerController::OnSkillWReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillEPressed()  { ShowSkillIndicator(ESkillSlot::E); }
void ARiftPlayerController::OnSkillEReleased() { FirePendingSkill(); }

void ARiftPlayerController::OnSkillR()
{
	RequestSkill(ESkillSlot::R);
}

void ARiftPlayerController::ShowSkillIndicator(ESkillSlot Slot)
{
	if (!OwnedChamp) { return; }

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

void ARiftPlayerController::Server_RequestBasicAttack_Implementation(ALoLCharacterBase* Target)
{
	/*ALoLChampion* Champ = Cast<ALoLChampion>(GetPawn());
	if (!Champ || !Target) { return; }
	Champ->StartAttackLoop(Target);*/
	
	if (!OwnedChamp || !Target) { return; }
	OwnedChamp->StartAttackLoop(Target);
}

void ARiftPlayerController::Server_MoveToLocation_Implementation(FVector Loc)
{
	if (OwnedChamp)
	{
		OwnedChamp->StopAttackLoop();
	}
	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Loc);
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
