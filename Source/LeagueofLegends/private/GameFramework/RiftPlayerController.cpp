#include "GameFramework/RiftPlayerController.h"

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
	
	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());
	
	// CameraActor 스폰
	CameraActor = GetWorld()->SpawnActor<ALoLCameraActor>(ALoLCameraActor::StaticClass());
    
	// 뷰타겟으로 설정
	SetViewTarget(CameraActor);
}

void ARiftPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	EdgeScrollWithMouse(DeltaTime);
}

void ARiftPlayerController::EdgeScrollWithMouse(float DeltaTime)
{
	float mouseX, mouseY;
	
	if (!GetMousePosition(mouseX, mouseY)) return;
	
	FVector2D viewportSize;
	if (GEngine && GEngine->GameViewport)
	{
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
		
		if (!CameraActor) return;
		
		FVector Delta(moveInput.Y * EdgeScrollSpeed * DeltaTime, moveInput.X * EdgeScrollSpeed * DeltaTime, 0.f);
		
		CameraActor->AddActorWorldOffset(Delta);
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
