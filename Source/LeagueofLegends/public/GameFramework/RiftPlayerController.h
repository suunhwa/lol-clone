#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/LoLCameraActor.h"
#include "GameFramework/RiftTypes.h"
#include "RiftPlayerController.generated.h"

class ALoLChampion;

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
	
	// Edge scrolling
	void EdgeScrollWithMouse(float DeltaTime);
	
	/*UPROPERTY(VisibleDefaultsOnly, Category = AI)
	TObjectPtr<UPathFollowingComponent> PathComp;
	
	UPROPERTY(EditAnywhere, Category="Input")
	float ShortPressThreshold;*/
	
	uint32 bMoveToMouseCursor : 1;
    	
	FVector CachedDestination;
    	
	float FollowTime = 0.0f;
	
	UPROPERTY()
	TObjectPtr<ALoLCameraActor> CameraActor;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float EdgeScrollSpeed = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float EdgeThreshold = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraInterpSpeed = 10.f;

	FVector TargetCameraLoc = FVector::ZeroVector;
};
