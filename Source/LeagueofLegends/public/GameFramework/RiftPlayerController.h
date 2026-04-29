#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftTypes.h"
#include "AStar/AStarGridManager.h"
#include "Components/SkillComponent.h"
#include "RiftPlayerController.generated.h"

class ALoLChampion;
class ALoLCharacterBase;
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

	UFUNCTION(Server, Reliable)
	void Server_RequestSkill(ESkillSlot Slot, FVector TargetLoc);

	UFUNCTION(Server, Reliable)
	void Server_RequestBasicAttack(ALoLCharacterBase* Target);
	
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
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_SkillQ;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_SkillW;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_SkillE;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_SkillR;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_Attack_A;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_LeftClick;

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
	void OnSkillQPressed();
	void OnSkillQReleased();
	void OnSkillWPressed();
	void OnSkillWReleased();
	void OnSkillEPressed();
	void OnSkillEReleased();
	void OnSkillR();
	void OnAPressed();
	void OnAReleased();
	void OnLeftClick();

	bool bAKeyPressed = false;

	// 스킬 조준 상태 (-1 = 없음, 0=Q, 1=W, 2=E, 3=R)
	int32 PendingSkillSlot = -1;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Indicators")
	TSubclassOf<AActor> CircleIndicatorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Indicators")
	TSubclassOf<AActor> LineIndicatorClass;

	UPROPERTY()
	TObjectPtr<AActor> CurrentIndicator;

	void ShowSkillIndicator(ESkillSlot Slot);
	void HideSkillIndicator();
	void UpdateIndicator();

private:
	void RequestSkill(ESkillSlot Slot);
	void FirePendingSkill();
	void TryBasicAttackAtCursor();
	
private:
	UPROPERTY()
	TObjectPtr<AAStarGridManager> GridManager;

};
