// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

class ULobbyUIWidget;
class ULobbyUIViewModel;
class USessionViewModel;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ALobbyHUD();
	
protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ULobbyUIWidget> LobbyUIClass;

private:
	UPROPERTY()
	TObjectPtr<ULobbyUIWidget> LobbyUIWidget;

	UPROPERTY()
	TObjectPtr<ULobbyUIViewModel> LobbyUIVM;

	UPROPERTY()
	TObjectPtr<USessionViewModel> SessionVM;
};
