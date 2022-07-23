// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawnController.h"
#include "PlayerCharacter.h"

void APlayerPawnController::MoveForward(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	//UE_LOG(LogTemp, Display, TEXT("MoveForward: %f"), Value);


	Move(EAxis::X, Value);
}

void APlayerPawnController::MoveRight(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	//UE_LOG(LogTemp, Display, TEXT("MoveRight: %f"), Value);


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
	if (Value == 0.0f)
	{
		return;
	}

	//UE_LOG(LogTemp, Display, TEXT("TurnX: %f"), Value);

	if (bMiddleMouseButtonPressed)
	{
		MoveHandByMouse(EAxis::X, Value);
	}
	else
	{
		AddYawInput(Value);
	}
}

void APlayerPawnController::TurnY(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	//UE_LOG(LogTemp, Display, TEXT("TurnY: %f"), Value);

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (PlayerCharacter)
	{
		if (bMiddleMouseButtonPressed)
		{
			MoveHandByMouse(EAxis::Z, Value * -1.0f);
		}
		else
		{
			FRotator Rotation = PlayerCharacter->Camera->GetRelativeRotation();

			Rotation.Pitch += Value;
			//Rotation.Pitch = FMath::Clamp(Rotation.Pitch, -90.0f, 90.0f);
			Rotation.Roll = 0.0f;

			PlayerCharacter->Camera->SetRelativeRotation(Rotation);

			if (!PlayerCharacter->IsVREnabled())
			{
				FRotator HandRotation = FRotator(90.0f, 0.0f, 0.0f);

				FRotator NewHandRotation = HandRotation + FRotator(Rotation.Pitch, 0.0f, 0.0f);

				PlayerCharacter->MotionControllerRight->SetRelativeRotation(NewHandRotation);
				PlayerCharacter->MotionControllerLeft->SetRelativeRotation(NewHandRotation);
			}
		}
	}
}

void APlayerPawnController::Jump()
{
	UE_LOG(LogTemp, Display, TEXT("Jump"));

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->Jump();


}

void APlayerPawnController::Crouch()
{
	UE_LOG(LogTemp, Display, TEXT("Crouch"));

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	if (PlayerCharacter->bIsCrouched)
	{
		PlayerCharacter->UnCrouch();
	}
	else
	{
		PlayerCharacter->Crouch();
	}
}

void APlayerPawnController::OnGrab(EHand Hand)
{
	if (Hand == EHand::Right)
	{
		bRightGrabPressed = true;
	}
	else
	{
		bLeftGrabPressed = true;
	}

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->OnGrab(Hand);
}

void APlayerPawnController::OnUnGrab(EHand Hand)
{
	if (Hand == EHand::Right)
	{
		bRightGrabPressed = false;
	}
	else
	{
		bLeftGrabPressed = false;
	}

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->OnUnGrab(Hand);
}

void APlayerPawnController::OnAction(EHand Hand)
{
	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	bool bGrabPressed = Hand == EHand::Right ? bRightGrabPressed : bLeftGrabPressed;

	PlayerCharacter->OnAction(Hand, bGrabPressed);
	PlayerCharacter->TriggerStateChanged(Hand, true);
}

void APlayerPawnController::OnTriggerReleased(EHand Hand)
{
	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->TriggerStateChanged(Hand, false);
}

bool APlayerPawnController::IsGrabButtonPressed(EHand Hand)
{
	return Hand == EHand::Right ? bRightGrabPressed : bLeftGrabPressed;
}

void APlayerPawnController::OnMiddleMousePressed()
{
	bMiddleMouseButtonPressed = true;
}

void APlayerPawnController::OnMiddleMouseReleased()
{
	bMiddleMouseButtonPressed = false;
}

void APlayerPawnController::MoveHandByMouse(EAxis::Type Axis, float Value)
{

#if !WITH_EDITOR
	return;
#endif

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());

	FRotator Rotation;

	auto* Controller = PlayerCharacter->GetMotionController(EHand::Right);

	if (PlayerCharacter)
	{
		Rotation = Controller->GetComponentRotation() - FRotator(90.0f, 0.0f, 0.0f);
	}

	Rotation.Roll = 0.0f;

	const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(Axis);

	//UE_LOG(LogTemp, Display, TEXT("[APlayerPawnController] MoveHandByMouse %d: %f. Direction: %s"), ((Axis == EAxis::Type::X)), Value, *Direction.ToString());

	Controller->AddLocalOffset(Direction * Value);
}

void APlayerPawnController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}

void APlayerPawnController::SetupInputComponent()
{
	Super::SetupInputComponent();

	this->InputComponent->BindAxis("MoveForward", this, &APlayerPawnController::MoveForward);
	this->InputComponent->BindAxis("MoveRight", this, &APlayerPawnController::MoveRight);

	this->InputComponent->BindAxis("TurnX", this, &APlayerPawnController::TurnX);
	this->InputComponent->BindAxis("TurnY", this, &APlayerPawnController::TurnY);

	this->InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &APlayerPawnController::Jump);
	this->InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &APlayerPawnController::Crouch);

	this->InputComponent->BindAction<FGrabDelegate>("Grab_Left", EInputEvent::IE_Pressed, this, &APlayerPawnController::OnGrab, EHand::Left);
	this->InputComponent->BindAction<FGrabDelegate>("Grab_Right", EInputEvent::IE_Pressed, this, &APlayerPawnController::OnGrab, EHand::Right);

	this->InputComponent->BindAction<FGrabDelegate>("Grab_Left", EInputEvent::IE_Released, this, &APlayerPawnController::OnUnGrab, EHand::Left);
	this->InputComponent->BindAction<FGrabDelegate>("Grab_Right", EInputEvent::IE_Released, this, &APlayerPawnController::OnUnGrab, EHand::Right);

	this->InputComponent->BindAction<FGrabDelegate>("Action_Left", EInputEvent::IE_Pressed, this, &APlayerPawnController::OnAction, EHand::Left);
	this->InputComponent->BindAction<FGrabDelegate>("Action_Right", EInputEvent::IE_Pressed, this, &APlayerPawnController::OnAction, EHand::Right);

	this->InputComponent->BindAction<FGrabDelegate>("Action_Left", EInputEvent::IE_Released, this, &APlayerPawnController::OnTriggerReleased, EHand::Left);
	this->InputComponent->BindAction<FGrabDelegate>("Action_Right", EInputEvent::IE_Released, this, &APlayerPawnController::OnTriggerReleased, EHand::Right);

	this->InputComponent->BindAction("MiddleMouse", EInputEvent::IE_Pressed, this, &APlayerPawnController::OnMiddleMousePressed);
	this->InputComponent->BindAction("MiddleMouse", EInputEvent::IE_Released, this, &APlayerPawnController::OnMiddleMouseReleased);

}

void APlayerPawnController::BeginPlay()
{
	Super::BeginPlay();
}
