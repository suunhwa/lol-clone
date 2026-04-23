// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/LoLCameraActor.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ALoLCameraActor::ALoLCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 2000.0f;
	CameraBoom->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	CameraBoom->bDoCollisionTest = false;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	
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

