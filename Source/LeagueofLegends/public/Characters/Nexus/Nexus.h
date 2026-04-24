#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Nexus.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ANexus : public AActor
{
	GENERATED_BODY()
    
public:    
	ANexus();

protected:
	virtual void BeginPlay() override;
	void Tick(float DeltaTime);

public:
	// 미니언이 때릴 수 있도록 체력 변수 정도 임시로 생성
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health = 5000.0f;
	
	// 데미지를 입었을 때 승패를 판단하는 함수
	void ReceiveDamage(float Damage);

private:
	bool bIsDestroyed = false;
};