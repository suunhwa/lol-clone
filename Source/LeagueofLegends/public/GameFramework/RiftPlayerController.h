#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftTypes.h"
#include "RiftPlayerController.generated.h"

class ALoLChampion;
class UInputMappingContext;
class UInputAction;

UCLASS()
class LEAGUEOFLEGENDS_API ARiftPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARiftPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(APawn* P) override;
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// 로비: 소환사 주문 선택 하게
	UFUNCTION(Server, Reliable)
	void Server_SelectSummonerSpells(ESummonerSpell Spell1, ESummonerSpell Spell2);

	// 로비: 라인 선택 하게 ?
	UFUNCTION(Server, Reliable)
	void Server_SelectLane(ELane Lane);
	
public:
	// ---------------------------------- Camera --------------------------------
	// Edge scrolling
	void EdgeScrollWithMouse(float DeltaTime);

	UPROPERTY()
	TObjectPtr<ALoLCameraActor> CameraActor;

	UPROPERTY()
	TObjectPtr<ALoLChampion> OwnedChamp;

	UPROPERTY(EditAnywhere, Category = "Camera|Settings")
	float EdgeScrollSpeed = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Settings")
	float EdgeThreshold = 50.f;

	UPROPERTY(EditAnywhere, Category = "Camera|Settings")
	float CameraInterpSpeed = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Camera|Bounds")
	bool bCameraBoundsEnabled = false;

	UPROPERTY(EditAnywhere, Category = "Camera|Bounds", meta = (EditCondition = "bCameraBoundsEnabled"))
	FVector2D CameraBoundsMin = FVector2D(-4000.f, -4000.f);

	UPROPERTY(EditAnywhere, Category = "Camera|Bounds", meta = (EditCondition = "bCameraBoundsEnabled"))
	FVector2D CameraBoundsMax = FVector2D(4000.f, 4000.f);;

	FVector TargetCameraLoc = FVector::ZeroVector;

protected:
	// ----------------------------------Input---------------------------------
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> MappingContexts;

	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_LockCam;

	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_FocusChamp;

	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, Category="Camera")
	TSubclassOf<ALoLCameraActor> CameraActorClass;

	virtual void SetupInputComponent() override;

public:
	bool bCameraLocked = false;

	// y키
	// 고정시점
	void OnCameraLockToggled();

	// space bar
	// 스페이스바 누르는 동안 챔피언 시점, 떼면 그 위치에서 멈춤
	void OnCameraFocusHeld();
	void OnCameraFocusReleased();

	void OnMove();
};
