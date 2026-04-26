// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LoLCameraActor.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ALoLCameraActor::ALoLCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 2000.0f;
	CameraBoom->SetRelativeRotation(FRotator(-55.0f, 130.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	CameraComp->FieldOfView = 70.0f;
}

// Called when the game starts or when spawned
void ALoLCameraActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALoLCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector ALoLCameraActor::GetViewForwardXY() const
{
	FVector Forward = CameraComp->GetForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	return Forward;
}

FVector ALoLCameraActor::GetViewRightXY() const
{
	FVector Right = CameraComp->GetRightVector();
	Right.Z = 0.f;
	Right.Normalize();
	return Right;
}
