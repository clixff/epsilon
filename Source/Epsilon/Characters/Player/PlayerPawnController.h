// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
	void MoveLeft(float Value);

	void Move(EAxis::Type Axis, float Value);

	void TurnX(float Value);
	void TurnY(float Value);

	void Jump();

protected:
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
};
