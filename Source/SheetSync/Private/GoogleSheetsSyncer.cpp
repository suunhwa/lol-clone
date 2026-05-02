// Fill out your copyright notice in the Description page of Project Settings.

#include "GoogleSheetsSyncer.h"

#include "DataTableEditorUtils.h"
#include "FileHelpers.h"
#include "HttpModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/DataTable.h"

UGoogleSheetsSyncer::UGoogleSheetsSyncer()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UGoogleSheetsSyncer::ParseURL(const FString& URL, FString& OutSpreadsheetId, FString& OutGid)
{
    // SpreadsheetId 추출
    // https://docs.google.com/spreadsheets/d/{ID}/edit?gid={GID}
    FString Left, Right;
    if (!URL.Split(TEXT("/spreadsheets/d/"), &Left, &Right))
    {
        UE_LOG(LogTemp, Error, TEXT("[SheetSync] URL 형식이 올바르지 않습니다: %s"), *URL);
        return false;
    }

    Right.Split(TEXT("/"), &OutSpreadsheetId, &Left);

    // gid 추출
    if (!Right.Split(TEXT("gid="), &Left, &OutGid))
    {
        UE_LOG(LogTemp, Error, TEXT("[SheetSync] gid를 찾을 수 없습니다: %s"), *URL);
        return false;
    }

    // gid 뒤에 붙는 불필요한 문자 제거
    FString GidOnly;
    OutGid.Split(TEXT("#"), &GidOnly, &Left);
    if (!GidOnly.IsEmpty())
    {
        OutGid = GidOnly;
    }

    return true;
}

void UGoogleSheetsSyncer::SyncAll()
{
    for (int32 i = 0; i < SyncEntries.Num(); i++)
    {
        RequestCSV(i);
    }
}

void UGoogleSheetsSyncer::RequestCSV(int32 Index)
{
    const FSheetSyncEntry& Entry = SyncEntries[Index];

    FString SpreadsheetId, Gid;
    if (!ParseURL(Entry.SheetURL, SpreadsheetId, Gid))
    {
        return;
    }

    FString URL = FString::Printf(
        TEXT("https://docs.google.com/spreadsheets/d/%s/export?format=csv&gid=%s"),
        *SpreadsheetId,
        *Gid);

    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("GET"));
    Request->OnProcessRequestComplete().BindLambda(
        [this, Index](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
        {
            if (!bSuccess || !Res.IsValid())
            {
                UE_LOG(LogTemp, Error,
                    TEXT("[SheetSync] 요청 실패 Index: %d"), Index);
                return;
            }

            OnCSVReceived(Index, Res->GetContentAsString());
        });

    Request->ProcessRequest();
}

void UGoogleSheetsSyncer::OnCSVReceived(int32 Index, const FString& CSVText)
{
    UDataTable* DataTable = SyncEntries[Index].TargetDataTable;
    if (!IsValid(DataTable))
    {
        UE_LOG(LogTemp, Error,
            TEXT("[SheetSync] DataTable이 유효하지 않습니다. Index: %d"), Index);
        return;
    }

    TArray<FString> Errors = DataTable->CreateTableFromCSVString(CSVText);
    if (Errors.Num() > 0)
    {
        for (const FString& Error : Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("[SheetSync] 파싱 에러: %s"), *Error);
        }
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("[SheetSync] 동기화 완료 Index: %d"), Index);
    
    // 에셋 변경 사항 즉시 반영
    if (DataTable->MarkPackageDirty())
    {
        UE_LOG(LogTemp, Log,
            TEXT("[SheetSync] DataTable 업데이트 완료 Index: %d"), Index);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[SheetSync] DataTable 업데이트 실패 Index: %d"), Index);
    }
    
    // 변경
    FDataTableEditorUtils::BroadcastPostChange(DataTable, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
    UE_LOG(LogTemp, Log, TEXT("[SheetSync] DataTable 뷰 갱신 완료 Index: %d"), Index);
    
    // 에셋 저장
    UEditorLoadingAndSavingUtils::SavePackages({ DataTable->GetOutermost() }, false);

    UE_LOG(LogTemp, Log, TEXT("[SheetSync] DataTable 저장 완료 Index: %d"), Index);
}