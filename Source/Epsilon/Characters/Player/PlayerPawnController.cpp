// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnController.h"
#include "PlayerCharacter.h"

void APlayerPawnController::MoveForward(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("MoveForward: %f"), Value);


	Move(EAxis::X, Value);
}

void APlayerPawnController::MoveLeft(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("MoveLeft: %f"), Value);


	Move(EAxis::Y, Value);
}

void APlayerPawnController::Move(EAxis::Type Axis, float Value)
{
	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	FRotator Rotation;

	if (PlayerCharacter)
	{
		Rotation = PlayerCharacter->Camera->GetComponentRotation();
	}

	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);

	GetPawn()->AddMovementInput(Direction, Value);
}

void APlayerPawnController::TurnX(float Value)
{
}

void APlayerPawnController::TurnY(float Value)
{
}

void APlayerPawnController::Jump()
{
	UE_LOG(LogTemp, Display, TEXT("Jump"));

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayHapticEffect(PlayerCharacter->ControllerVibrationCurve, EControllerHand::Right, 1.0f, false);

	//PlayDynamicForceFeedback(1.0f, 1.0f, true, true, true, true);
}

void APlayerPawnController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void APlayerPawnController::SetupInputComponent()
{
	Super::SetupInputComponent();

	this->InputComponent->BindAxis("MoveForward", this, &APlayerPawnController::MoveForward);
	this->InputComponent->BindAxis("MoveLeft", this, &APlayerPawnController::MoveLeft);

	this->InputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &APlayerPawnController::Jump);

}

void APlayerPawnController::BeginPlay()
{
	Super::BeginPlay();
}
