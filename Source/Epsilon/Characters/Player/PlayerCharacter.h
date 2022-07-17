// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "../../Components/GrabComponent.h"
#include "../../Misc/Misc.h"
#include "Components/BoxComponent.h"
#include "../../World/GrabActor.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class EPSILON_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UCameraComponent* Camera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UMotionControllerComponent* MotionControllerLeft = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UMotionControllerComponent* MotionControllerRight = nullptr;

	UPROPERTY(EditAnywhere)
		UHapticFeedbackEffect_Base* ControllerVibrationCurve;

	UPROPERTY(EditAnywhere)
		FVector FingerTraceOffset;

	UPROPERTY(VisibleAnywhere)
		UGrabComponent* GrabComponentRight = nullptr;

	UPROPERTY(VisibleAnywhere)
		UGrabComponent* GrabComponentLeft = nullptr;

	void OnGrab(EHand Hand);

	void OnUnGrab(EHand Hand);

	FVector GetDeltaControllerPosition(EHand Hand);

	void OnAction(EHand Hand, bool bGrabPressed);

	UPROPERTY(VisibleAnywhere)
		UGrabComponent* GrabComponentFlyingRight = nullptr;

	UPROPERTY(VisibleAnywhere)
		UGrabComponent* GrabComponentFlyingLeft = nullptr;

	UPROPERTY(EditAnywhere)
		UBoxComponent* FistCollisionRight = nullptr;

	UPROPERTY(EditAnywhere)
		UBoxComponent* FistCollisionLeft = nullptr;

	UPROPERTY(EditAnywhere)
		USceneComponent* VROrigin = nullptr;

	void SetFistCollisionEnabled(EHand Hand, bool bEnabled);

	void UpdateBodyPositionInVR();

	void DetachItemFromHand(EHand Hand);

	UPROPERTY(EditAnywhere)
		TSubclassOf<AGrabActor> GrabActorToSpawn = nullptr;

	void SpawnActor(EHand Hand);
private:
	void TraceFromFinger(EHand Hand);

	TArray<FVector> ControllerRightDeltas;
	TArray<FVector> ControllerLeftDeltas;

	int32 MaxControllerPositions = 15;

	void UpdateCachedControllerPosition(EHand Hand, float DeltaTime);

	FVector FingerTraceLeft;
	FVector FingerTraceRight;

	FVector ControllerRightLastTickLocation;
	FVector ControllerLeftLastTickLocation;
};
