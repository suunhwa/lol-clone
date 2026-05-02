#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GoogleSheetsSyncer.generated.h"

USTRUCT(BlueprintType)
struct FSheetSyncEntry
{
	GENERATED_BODY()

	// 구글 시트 URL 전체 붙여넣기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SheetSync")
	FString SheetURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SheetSync")
	UDataTable* TargetDataTable = nullptr;
};

UCLASS(ClassGroup=(SheetSync), meta=(BlueprintSpawnableComponent))
class SHEETSYNC_API UGoogleSheetsSyncer : public UActorComponent
{
	GENERATED_BODY()

public:
	UGoogleSheetsSyncer();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SheetSync")
	TArray<FSheetSyncEntry> SyncEntries;

	// 전체 동기화 해보자잇
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "SheetSync")
	void SyncAll();

private:
	bool ParseURL(const FString& URL, FString& OutSpreadsheetId, FString& OutGid);
	void RequestCSV(int32 Index);
	void OnCSVReceived(int32 Index, const FString& CSVText);
};