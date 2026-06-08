// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "SheetSyncSettings.generated.h"

USTRUCT(BlueprintType)
struct FSheetSyncEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SheetSync")
	FString SheetURL;

	UPROPERTY(EditAnywhere, Category = "SheetSync")
	TSoftObjectPtr<UDataTable> TargetDataTable;
};

USTRUCT(BlueprintType)
struct FSheetSyncCategory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SheetSync")
	FString CategoryName;

	UPROPERTY(EditAnywhere, Category = "SheetSync")
	TArray<FSheetSyncEntry> Entries;
};

UCLASS(Config = SheetSync, DefaultConfig, meta = (DisplayName = "Sheet Syncer"))
class SHEETSYNC_API USheetSyncSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// UFUNCTION(CallInEditor, Category = "SheetSync")
	// void SyncAll();
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "SheetSync")
	TArray<FSheetSyncCategory> Categories;

	static const USheetSyncSettings* Get()
	{
		return GetDefault<USheetSyncSettings>();
	}

	virtual FName GetCategoryName() const override
	{
		return TEXT("Plugins");
	}
};
