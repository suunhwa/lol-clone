#include "UI/View/HPBarWidget.h"

#include "Components/ObjectStatComponent.h"
#include "Components/ProgressBar.h"
#include "Components/StatComponent.h"

void UHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HPBar)
	{
		HPBar->SetPercent(1.f);
	}
}

void UHPBarWidget::InitWidget(UStatComponent* InStatComp)
{
	if (!InStatComp) { return; }

	StatComp = InStatComp;
	StatComp->OnHPChanged.AddUObject(this, &UHPBarWidget::OnHPChanged);

	if (HPBar && StatComp->GetMaxHP() > 0.f)
	{
		HPBar->SetPercent(StatComp->GetCurrentHP() / StatComp->GetMaxHP());
	}
}
void UHPBarWidget::InitWidgetFromDamageable(AActor* InActor)
{
	if (!InActor) return;

	// 방법 A: 인터페이스를 구현한 액터인지 확인하고 델리게이트 연결
	// (만약 포탑의 OnHPChanged와 미니언의 OnHPChanged가 같은 시그니처라면 가능)
    
	// 방법 B: 가장 확실한 방법 - 그냥 포탑 컴포넌트를 직접 체크
	if (auto* OSC = InActor->FindComponentByClass<UObjectStatComponent>())
	{
		OSC->OnHPChanged.AddUObject(this, &UHPBarWidget::OnHPChanged);
		// 초기 값 세팅
		OnHPChanged(OSC->GetCurrentHP(), OSC->GetMaxHP());
	}
}

void UHPBarWidget::OnHPChanged(float Current, float Max)
{
	if (!HPBar) { return; }
	
	// Max가 0 이하일 때 예외처리 및 비율을 0.0 ~ 1.0 사이로 제한
	float Percent = (Max > 0.f) ? (Current / Max) : 0.f;
	HPBar->SetPercent(Max > 0.f ? Current / Max : 0.f);
}

