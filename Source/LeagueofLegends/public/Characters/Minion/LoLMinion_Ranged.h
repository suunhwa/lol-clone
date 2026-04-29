#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLMinion.h"
#include "LoLMinion_Ranged.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLMinion_Ranged : public ALoLMinion
{
	GENERATED_BODY()

public:
	ALoLMinion_Ranged();

protected:
	// 부모의 공격 함수를 오버라이드하여 투사체 발사 로직을 넣습니다.
	virtual void PerformAttack() override;

	// 블루프린트에서 어떤 투사체를 발사할지 선택합니다 (예: BP_Minion_Projectile)
	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	TSubclassOf<class AActor> ProjectileClass;

	// 투사체가 스폰될 위치 (보통 지팡이나 손 끝)
	UPROPERTY(EditAnywhere, Category = "AI|Combat")
	FVector ProjectileOffset = FVector(50.f, 0.f, 60.f);
};