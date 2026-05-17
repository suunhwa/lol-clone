#include "UI/View/RoomInfoWidget.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"

void URoomInfoWidget::SetInfo(const FLoLSessionInfo& Info)
{
	SessionIndex = Info.Index;

	if (Txt_RoomName)
	{
		Txt_RoomName->SetText(FText::FromString(Info.RoomName));
	}
	if (Txt_HostName)
	{
		Txt_HostName->SetText(FText::FromString(Info.HostName));
	}
	if (Txt_PlayerCount)
	{
		Txt_PlayerCount->SetText(FText::FromString(Info.MaxPlayer));
	}
	
}

void URoomInfoWidget::SetSelected(bool bSelected)
{
	if (Cbx_SelectRoom)
	{
		Cbx_SelectRoom->SetIsChecked(bSelected);
	}
		
}

FReply URoomInfoWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnSelected.ExecuteIfBound(SessionIndex, this);
	return FReply::Handled();
}
