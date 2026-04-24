// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Nexus/Nexus.h"


// Sets default values
ANexus::ANexus()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Tags.Add(TEXT("Structure"));
}

// Called when the game starts or when spawned
void ANexus::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANexus::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANexus::ReceiveDamage(float Damage)
{
	if (bIsDestroyed) return;

	Health -= Damage;

	if (Health <= 0.0f)
	{
		bIsDestroyed = true;
		Health = 0.0f;

		// 자신의 팀 태그 찾기 (보통 태그의 두 번째나 어딘가에 팀 이름이 있음)
		FString MyTeam = TEXT("Unknown");
		FString EnemyTeam = TEXT("Unknown");

		// 태그를 순회하며 팀 이름 찾기 (BlueTeam 또는 RedTeam)
		for (const FName& Tag : Tags)
		{
			if (Tag.ToString().Contains(TEXT("BlueTeam")))
			{
				MyTeam = TEXT("BlueTeam");
				EnemyTeam = TEXT("RedTeam");
				break;
			}
			else if (Tag.ToString().Contains(TEXT("RedTeam")))
			{
				MyTeam = TEXT("RedTeam");
				EnemyTeam = TEXT("BlueTeam");
				break;
			}
		}

		// --- 승패 로그 출력 ---
		UE_LOG(LogTemp, Error, TEXT("========================================"));
		UE_LOG(LogTemp, Error, TEXT("   [ %s ] 넥서스가 파괴되었습니다!   "), *MyTeam);
		UE_LOG(LogTemp, Error, TEXT("   >>> 결과: %s 승리 / %s 패배 <<<   "), *EnemyTeam, *MyTeam);
		UE_LOG(LogTemp, Error, TEXT("========================================"));

		// 실제 게임이라면 여기서 게임 오버 UI를 띄우거나 게임을 멈춤
		// Destroy(); // 넥서스 액터를 제거하고 싶다면 주석 해제
	}
}

