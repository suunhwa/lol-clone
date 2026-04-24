#include "GameFramework/RiftPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "Characters/LoLChampion.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftPlayerState.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

ARiftPlayerController::ARiftPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
}

void ARiftPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());
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
			UE_LOG(LogTemp, Warning, TEXT("Left***"));
			moveInput.X = -1.f;
		}
		if (mouseX > viewportSize.X - EdgeThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("Right***"));
			moveInput.X = 1.f;
		}
		if (mouseY < EdgeThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("Top***"));
			moveInput.Y = 1.f;
		}
		if (mouseY > viewportSize.Y - EdgeThreshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("Bottom***"));
			moveInput.Y = -1.f;
		}

		if (moveInput.IsNearlyZero()) return;

		FVector Forward = CameraActor->GetActorForwardVector();
		FVector Right = CameraActor->GetActorRightVector();
		Forward.Z = 0.f;
		Forward.Normalize();
		Right.Z = 0.f;
		Right.Normalize();

		FVector Delta = (Forward * moveInput.Y + Right * moveInput.X) * EdgeScrollSpeed * DeltaTime;

		UE_LOG(LogTemp,
		       Warning,
		       TEXT("***Delta: %s | CamLoc: %s"),
		       *Delta.ToString(),
		       *CameraActor->GetActorLocation().ToString());

		// CameraActor->AddActorWorldOffset(Delta);
		TargetCameraLoc += Delta;
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
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Started, this, &ARiftPlayerController::OnCameraFocusStarted);
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Triggered, this, &ARiftPlayerController::OnCameraFocusHeld);
	EIC->BindAction(IA_FocusChamp, ETriggerEvent::Completed, this, &ARiftPlayerController::OnCameraFocusReleased);
	EIC->BindAction(IA_Move, ETriggerEvent::Started, this, &ARiftPlayerController::OnMove);
}

void ARiftPlayerController::OnCameraFocusStarted()
{
	PreFocusCameraLoc = TargetCameraLoc;
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
	if (!bCameraLocked)
	{
		TargetCameraLoc = PreFocusCameraLoc;
	}
}

void ARiftPlayerController::OnMove()
{
	UE_LOG(LogTemp, Warning, TEXT("*컨트롤러: %s"), *GetNameSafe(this));
	UE_LOG(LogTemp, Warning, TEXT("*Pawn: %s"), *GetNameSafe(GetPawn()));
	UE_LOG(LogTemp, Warning, TEXT("*OwnedChamp: %s"), *GetNameSafe(OwnedChamp));

	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
	if (HitResult.bBlockingHit)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.ImpactPoint);
	}
}

void ARiftPlayerController::OnCameraLockToggled()
{
	bCameraLocked = !bCameraLocked;
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
