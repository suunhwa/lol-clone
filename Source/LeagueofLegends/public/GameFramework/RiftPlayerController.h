#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/RiftPlayerCameraManager.h"
#include "Type/RiftTypes.h"
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
	virtual void SeamlessTravelTo(APlayerController* NewPC) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void AcknowledgePossession(APawn* P) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(Server, Reliable)
	void Server_SelectTeam(ETeam Team);

	UFUNCTION(Server, Reliable)
	void Server_SelectChampion(FName ChampionID);

	UFUNCTION(Server, Reliable)
	void Server_SetReady();

	UFUNCTION(Server, Reliable)
	void Server_StartChampionSelect();

	UFUNCTION(Server, Reliable)
	void Server_StartGame();
	
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
	void Server_RequestBasicAttack(AActor* Target);

	UFUNCTION(Server, Reliable)
	void Server_MoveToLocation(FVector Loc);
	
public:
	// ---------------------------------- Camera --------------------------------
	void EdgeScrollWithMouse(float DeltaTime);

	ARiftPlayerCameraManager* GetRiftCameraManager() const
	{
		return Cast<ARiftPlayerCameraManager>(PlayerCameraManager);
	}

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
	FVector2D CameraBoundsMin = FVector2D(-6000.f, -6000.f);

	UPROPERTY(EditAnywhere, Category = "Camera|Bounds", meta = (EditCondition = "bCameraBoundsEnabled"))
	FVector2D CameraBoundsMax = FVector2D(6000.f, 6000.f);;

	FVector TargetCameraLoc = FVector::ZeroVector;
	FTimerHandle CameraInitTimer;

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

	UPROPERTY(EditAnywhere, Category="Input|Input Actions")
	TObjectPtr<UInputAction> IA_Shop;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_Exit;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_Spell_D;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions")
	TObjectPtr<UInputAction> IA_Spell_F;
	
	UPROPERTY(EditAnywhere, Category ="Input|Input Actions|Debug")
	TObjectPtr<UInputAction> IA_LevelUp;


	virtual void SetupInputComponent() override;

public:
	bool bCameraLocked = false;
	bool bCameraInitialized = false;
	bool bIsRedTeam = false; // 스폰 위치 기준으로 설정됨

	// 클릭 이동 (Nav Mesh 경로 → AddMovementInput으로 구동)
	TArray<FVector> MovePath;
	int32 MovePathIndex = 0;
	bool bHasMoveTarget = false;
	FVector MoveTargetLocation = FVector::ZeroVector;
	static constexpr float MoveAcceptanceRadius = 80.f;

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
	void OnSkillRPressed();
	void OnSkillRReleased();
	void OnAPressed();
	void OnAReleased();
	void OnLeftClick();
	void OnToggleShop();
	void OnExit();
	void OnSpellD();
	void OnSpellF();

	UFUNCTION(Server, Reliable)
	void Server_CastSummonerSpell(int32 SlotIndex, FVector TargetLoc);

	UFUNCTION(Client, Reliable)
	void Client_OnSpellCast(int32 SlotIndex, float Cooldown);

	bool bAKeyPressed = false;


	UFUNCTION(Server, Reliable)
	void Server_AddXP();

	UFUNCTION(Server, Reliable)
	void Server_AssignSkillPoint(ESkillSlot Slot);

	UFUNCTION(Client, Reliable)
	void Client_OnSkillAssigned(ESkillSlot Slot);

	// 스킬 조준 상태 (-1 = 없음, 0=Q, 1=W, 2=E, 3=R)
	int32 PendingSkillSlot = -1;

	// 소환사 주문 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "SummonerSpell")
	TObjectPtr<UDataTable> SpellBaseTable;

	UPROPERTY(EditDefaultsOnly, Category = "SummonerSpell")
	TObjectPtr<UDataTable> SpellTargetingTable;

	UPROPERTY(EditDefaultsOnly, Category = "SummonerSpell")
	TObjectPtr<UDataTable> SpellSecondaryEffectTable;

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
