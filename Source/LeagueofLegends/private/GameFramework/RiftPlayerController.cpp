#include "GameFramework/RiftPlayerController.h"

#include "Characters/LoLChampion.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftPlayerState.h"

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
	if (!Champion) return;

	FVector CameraStartLoc = Champion->GetActorLocation();
	CameraActor = GetWorld()->SpawnActor<ALoLCameraActor>(ALoLCameraActor::StaticClass(), FTransform(FRotator::ZeroRotator, CameraStartLoc));
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

	EdgeScrollWithMouse(DeltaTime);

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
		Forward.Z = 0.f; Forward.Normalize();
		Right.Z = 0.f; Right.Normalize();

		FVector Delta = (Forward * moveInput.Y + Right * moveInput.X) * EdgeScrollSpeed * DeltaTime;
		
		UE_LOG(LogTemp, Warning, TEXT("***Delta: %s | CamLoc: %s"), *Delta.ToString(), *CameraActor->GetActorLocation().ToString());
		
		// CameraActor->AddActorWorldOffset(Delta);
		TargetCameraLoc += Delta;
	}
	
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
