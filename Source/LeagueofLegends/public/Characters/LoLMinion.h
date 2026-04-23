// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "LoLMinion.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLMinion();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	

protected:
	// 플레이어를 타겟으로 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AActor* TargetPlayer;
	
	// 이동속도 // 나중에 데이터테이블에서 가져오기 전까지 사용할 임시값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MoveSpeed = 300.0f;
	
public:
	// 타겟 갱신 함수
	UFUNCTION()
	void UpdateTarget();
	
protected:
	// A*로 찾아낸 경로 지점들
	TArray<FVector> CurrentPath;
	
	// 타겟 업데이트 타이머 핸들
	FTimerHandle TargetUpdateTimerHandle;
	
	UPROPERTY()
	class AAStarGridManager* GridManager;
};


