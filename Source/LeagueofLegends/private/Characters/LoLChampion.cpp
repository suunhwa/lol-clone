// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/LoLChampion.h"

#include "LeagueofLegends.h"
#include "Characters/Data/ChampionData.h"
#include "Components/StatComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkillComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Struct/ChampionStatStruct.h"

ALoLChampion::ALoLChampion()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALoLChampion::BeginPlay()
{
	Super::BeginPlay();
	
	PRINTLOG_SH(TEXT("LoLChampion BeginPlay***"));
	
	PRINTLOG_SH(TEXT("SkillComp: %s"), *GetNameSafe(SkillComp));
	if (SkillComp)
	{
		SkillComp->OnSkillActivated.AddUObject(this, &ALoLChampion::HandleSkillActivated);

		// 테스트용: 모든 스킬 랭크 1로 설정
		// TODO: 레벨업 시 플레이어가 직접 할당하도록 변경 예정
		bool bQ = SkillComp->AssignSkillPoint(ESkillSlot::Q);
		bool bW = SkillComp->AssignSkillPoint(ESkillSlot::W);
		bool bE = SkillComp->AssignSkillPoint(ESkillSlot::E);
		bool bR = SkillComp->AssignSkillPoint(ESkillSlot::R);
		PRINTLOG_SH(TEXT("AssignSkillPoint — Q:%d W:%d E:%d R:%d"), bQ, bW, bE, bR);
	}

	if (!ChampionData) return;

	InitVisuals();

	if (HasAuthority())
	{
		InitStats();
	}
}

void ALoLChampion::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALoLChampion, ChampionData);
}

void ALoLChampion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALoLChampion::OnRep_ChampionData()
{
	InitVisuals();
}

void ALoLChampion::InitVisuals()
{
	if (!ChampionData) return;

	if (ChampionData->Mesh)
	{
		GetMesh()->SetSkeletalMesh(ChampionData->Mesh);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

	if (ChampionData->AnimBP)
	{
		GetMesh()->SetAnimInstanceClass(ChampionData->AnimBP);
	}
}

// data table row name에 ChampionID 포함된 첫 번째 행 반환
template <typename T>
static const T* FindRowByChampionID(UDataTable* Table, const FName& ChampionID)
{
	if (!Table) return nullptr;

	const FString id = ChampionID.ToString();
	for (const FName& RowName : Table->GetRowNames())
	{
		if (RowName.ToString().Contains(id, ESearchCase::IgnoreCase))
		{
			return Table->FindRow<T>(RowName, TEXT(""));
		}
	}
	return nullptr;
}

// data table row name에 ChampionID 포함된 모든 행 반환 (skill관련 table etc)
template <typename T>
static TArray<const T*> FindAllRowsByChampionID(UDataTable* Table, const FName& ChampionID)
{
	TArray<const T*> Result;
	if (!Table) return Result;

	const FString id = ChampionID.ToString();
	for (const FName& RowName : Table->GetRowNames())
	{
		if (RowName.ToString().Contains(id, ESearchCase::IgnoreCase))
		{
			if (const T* Row = Table->FindRow<T>(RowName, TEXT("")))
			{
				Result.Add(Row);
			}
		}
	}

	return Result;
}


void ALoLChampion::InitStats()
{
	if (!ChampionData || !StatComp) return;

	// BaseTable : Row name = ChampionID
	const FChampionBaseRow* baseRow = FindRowByChampionID<FChampionBaseRow>(
		ChampionData->BaseTable,
		ChampionData->ChampionID);
	const FChampionStatRow* statRow = FindRowByChampionID<FChampionStatRow>(
		ChampionData->StatTable,
		ChampionData->ChampionID);
	const FChampionGrowthRow* growthRow = FindRowByChampionID<FChampionGrowthRow>(
		ChampionData->GrowthTable,
		ChampionData->ChampionID);

	if (!baseRow || !statRow || !growthRow)
	{
		PRINTLOG_SH(TEXT("[%s] 테이블 Row 못 찾음 — Base:%d Stat:%d Growth:%d"),
		            *ChampionData->ChampionID.ToString(),
		            !!baseRow,
		            !!statRow,
		            !!growthRow);
		return;
	}

	StatComp->InitStats(*baseRow, *statRow, *growthRow);

	// moveSpeed characterMovement에 반영
	GetCharacterMovement()->MaxWalkSpeed = StatComp->GetMoveSpeed();

	PRINTLOG_SH(TEXT("[%s] InitStats Succeeded - HP: %.f Mana: %.f AD: %.1f Armor: %.1f MoveSpeed: %.0f"),
	            *ChampionData->ChampionID.ToString(),
	            StatComp->GetCurrentHP(),
	            StatComp->GetCurrentMana(),
	            StatComp->GetAD(),
	            StatComp->GetArmor(),
	            StatComp->GetMoveSpeed());
}

void ALoLChampion::HandleSkillActivated(ESkillSlot Slot, FVector TargetLocation)
{
	switch (Slot)
	{
	case ESkillSlot::Q:
		PRINTLOG_SH(TEXT("Q 발동 → %s"), *TargetLocation.ToString());
		// TODO: 이즈리얼 Q 구현
		break;
	case ESkillSlot::W:
		PRINTLOG_SH(TEXT("W 발동"));
		break;
	case ESkillSlot::E:
		PRINTLOG_SH(TEXT("E 발동"));
		break;
	case ESkillSlot::R:
		PRINTLOG_SH(TEXT("R 발동"));
		break;
	}
}