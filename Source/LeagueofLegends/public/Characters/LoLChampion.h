// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/LoLCharacterBase.h"
#include "Components/SkillComponent.h"
#include "Data/ChampionData.h"
#include "LoLChampion.generated.h"

class UInventoryComponent;
class UStatModifierComponent;

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
	
public:
#pragma region Component Getters
	UStatModifierComponent* GetStatModifierComp() const { return StatModifierComp; }
	UInventoryComponent* GetInventoryComp() const { return InventoryComp; }
#pragma endregion
	
private:
	void InitVisuals();
	void InitStats();
	
	void HandleSkillActivated(ESkillSlot Slot, FVector TargetLoc);
	
	UFUNCTION()
	void OnRep_ChampionData();
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStatModifierComponent> StatModifierComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComp;
};
