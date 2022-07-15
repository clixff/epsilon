// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../../Misc/Misc.h"
#include "PlayerPawnController.generated.h"

/**
 * 
 */
UCLASS()
class EPSILON_API APlayerPawnController : public APlayerController
{
	GENERATED_BODY()
public:
	void MoveForward(float Value);
	void MoveRight(float Value);

	void Move(EAxis::Type Axis, float Value);

	void TurnX(float Value);
	void TurnY(float Value);

	void Jump();

	void Crouch();

	void OnGrab(EHand Hand);
	DECLARE_DELEGATE_OneParam(FGrabDelegate, EHand)

	void OnUnGrab(EHand Hand);

	void OnAction(EHand Hand);
protected:
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;

	bool bLeftGrabPressed = false;
	bool bRightGrabPressed = false;
};
