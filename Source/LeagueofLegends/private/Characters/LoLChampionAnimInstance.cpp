#include "Characters/LoLChampionAnimInstance.h"

#include "Characters/LoLCharacterBase.h"
#include "Components/StateComponent.h"

void ULoLChampionAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OwnerChar = Cast<ALoLCharacterBase>(GetOwningActor());
	if (!OwnerChar) { return; }

	StateComp = OwnerChar->StateComp;
}

void ULoLChampionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerChar || !StateComp) { return; }

	CurrentState = StateComp->GetCurrentState();

	MoveSpeed = OwnerChar->GetVelocity().Size2D();
	bIsMoving = MoveSpeed > 10.f;
	bIsAttacking = (CurrentState == ECharacterState::BasicAttacking);
	bIsCastingSkill = (CurrentState == ECharacterState::CastingSkill);
	bIsHit = (CurrentState == ECharacterState::Hit);
	bIsDead = (CurrentState == ECharacterState::Dead);
}
