#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/LoLSessionSubsystem.h"
#include "Input/Reply.h"
#include "RoomInfoWidget.generated.h"

class UTextBlock;
class UButton;
class UCheckBox;

DECLARE_DELEGATE_TwoParams(FOnRoomInfoSelected, int32 /*SessionIndex*/, class URoomInfoWidget* /*ThisWidget*/);

UCLASS()
class LEAGUEOFLEGENDS_API URoomInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetInfo(const FLoLSessionInfo& Info);

	// 선택 상태 외부에서 제어 (SessionWidget이 라디오 그룹처럼 관리)
	void SetSelected(bool bSelected);

	FOnRoomInfoSelected OnSelected;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> Cbx_SelectRoom;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_RoomName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_HostName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_MapName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_PlayerCount;

	// 위젯 전체 클릭 감지 (btn_slot 없이 슬롯 전체가 클릭 영역)
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	int32 SessionIndex = -1;
};
