#include "GameFramework/RiftPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "LeagueofLegends.h"
#include "AStar/AStarGridManager.h"
#include "Engine/LocalPlayer.h"
#include "Characters/LoLChampion.h"
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

	if (!IsLocalController() || !CameraActor) return;

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
	if (!CameraActor) return;

	float mouseX, mouseY;

	if (!GetMousePosition(mouseX, mouseY)) return;

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

		if (moveInput.IsNearlyZero()) return;

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
	if (!EIC) return;

	EIC->BindAction(IA_LockCam, ETriggerEvent::Started, this, &ARiftPlayerController::OnCameraLockToggled);
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Triggered, this, &ARiftPlayerController::OnCameraFocusHeld);
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Completed, this, &ARiftPlayerController::OnCameraFocusReleased);
	EIC->BindAction(IA_Move, ETriggerEvent::Started, this, &ARiftPlayerController::OnMove);

	EIC->BindAction(IA_SkillQ, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillQ);
	EIC->BindAction(IA_SkillW, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillW);
	EIC->BindAction(IA_SkillE, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillE);
	EIC->BindAction(IA_SkillR, ETriggerEvent::Started, this, &ARiftPlayerController::OnSkillR);
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
	/*PRINTLOG_SH(TEXT("*컨트롤러: %s"), *GetNameSafe(this));
	PRINTLOG_SH(TEXT("*Pawn: %s"), *GetNameSafe(GetPawn()));
	PRINTLOG_SH(TEXT("*OwnedChamp: %s"), *GetNameSafe(OwnedChamp));*/

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if (!HitResult.bBlockingHit) return;

	FVector Dir = HitResult.ImpactPoint - OwnedChamp->GetActorLocation();
	Dir.Z = 0.f;
	if (!Dir.IsNearlyZero())
	{
		OwnedChamp->SetActorRotation(Dir.ToOrientationRotator());
	}

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.ImpactPoint);
}

void ARiftPlayerController::OnCameraLockToggled()
{
	bCameraLocked = !bCameraLocked;
}

void ARiftPlayerController::OnAPressed()
{
	bCameraLocked = true;
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
	if (!OwnedChamp) return;

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
			if (Actor == OwnedChamp) continue;

			UStatComponent* Stat = Actor->FindComponentByClass<UStatComponent>();
			if (!Stat || Stat->IsDead()) continue;

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
		OwnedChamp->CombatComp->PerformBasicAttack(Target);
		PRINTLOG_SH(TEXT("A+LMB 평타 -> %s"), *GetNameSafe(Target));
	}
}

void ARiftPlayerController::OnSkillQ()
{
	PRINTLOG_SH(TEXT("[Q] OwnedChamp: %s"), *GetNameSafe(OwnedChamp));
	RequestSkill(ESkillSlot::Q);
}

void ARiftPlayerController::OnSkillW()
{
	PRINTLOG_SH(TEXT("[W] OwnedChamp: %s"), *GetNameSafe(OwnedChamp));
	RequestSkill(ESkillSlot::W);
}

void ARiftPlayerController::OnSkillE()
{
	PRINTLOG_SH(TEXT("[E] OwnedChamp: %s"), *GetNameSafe(OwnedChamp));
	RequestSkill(ESkillSlot::E);
}

void ARiftPlayerController::OnSkillR()
{
	PRINTLOG_SH(TEXT("[R] OwnedChamp: %s"), *GetNameSafe(OwnedChamp));
	RequestSkill(ESkillSlot::R);
}

void ARiftPlayerController::RequestSkill(ESkillSlot Slot)
{
	if (!OwnedChamp || !OwnedChamp->SkillComp) return;

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	FVector targetLoc = HitResult.bBlockingHit
		                    ? HitResult.ImpactPoint
		                    : OwnedChamp->GetActorLocation() + OwnedChamp->GetActorForwardVector() * 1000.f;

	OwnedChamp->SkillComp->RequestActivateSkill(Slot, targetLoc);
}

void ARiftPlayerController::Server_SelectSummonerSpells_Implementation(ESummonerSpell Spell1, ESummonerSpell Spell2)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) return;

	PS->SetSummonerSpells(Spell1, Spell2);
}

void ARiftPlayerController::Server_SelectLane_Implementation(ELane Lane)
{
	ARiftPlayerState* PS = GetPlayerState<ARiftPlayerState>();
	if (!PS) return;

	PS->SetLane(Lane);
}
