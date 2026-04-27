// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "Data/ChampionData.h"
#include "LoLChampion.generated.h"

UCLASS()
class LEAGUEOFLEGENDS_API ALoLChampion : public ALoLCharacterBase
{
	GENERATED_BODY()

public:
	ALoLChampion();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_ChampionData, EditDefaultsOnly, BlueprintReadOnly, Category = "Champion")
	TObjectPtr<UChampionData> ChampionData;

public:
	virtual void Tick(float DeltaTime) override;
	
private:
	void InitVisuals();
	void InitStats();
	
	UFUNCTION()
	void OnRep_ChampionData();
};
