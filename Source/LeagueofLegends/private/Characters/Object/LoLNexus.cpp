#include "Characters/Object/LoLNexus.h"

ALoLNexus::ALoLNexus()
{
	ObjectID = 11301;
}

void ALoLNexus::OnDestroyed()
{
	Super::OnDestroyed();
	// Game Over 로직 실행
}