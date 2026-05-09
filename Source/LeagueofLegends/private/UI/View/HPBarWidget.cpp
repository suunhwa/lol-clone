#include "UI/View/HPBarWidget.h"

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

void UHPBarWidget::OnHPChanged(float Current, float Max)
{
	if (!HPBar) { return; }

	HPBar->SetPercent(Max > 0.f ? Current / Max : 0.f);
}

