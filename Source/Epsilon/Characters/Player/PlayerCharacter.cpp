// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	Camera->SetupAttachment(GetRootComponent());

	MotionControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Left"));
	MotionControllerLeft->SetupAttachment(GetRootComponent());
	MotionControllerLeft->SetTrackingSource(EControllerHand::Left);

	MotionControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Right"));
	MotionControllerRight->SetupAttachment(GetRootComponent());
	MotionControllerRight->SetTrackingSource(EControllerHand::Right);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	bool bVREnabled = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (bVREnabled)
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	}

	UE_LOG(LogTemp, Display, TEXT("[APlayerPawn] bVREnabled: %d"), bVREnabled);

	Cast<APlayerController>(GetController())->ConsoleCommand(TEXT("stat unit"));
	Cast<APlayerController>(GetController())->ConsoleCommand(TEXT("stat fps"));
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

