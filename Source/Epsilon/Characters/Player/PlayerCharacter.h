// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "../../Components/GrabComponent.h"
#include "../../Misc/Misc.h"
#include "Components/BoxComponent.h"
#include "../../World/Grab/GrabActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
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
		UHapticFeedbackEffect_Base* ControllerVibrationPhysics;

	UPROPERTY(EditAnywhere)
		UHapticFeedbackEffect_Base* ControllerVibrationGrab;

	UPROPERTY(EditAnywhere)
		FVector FingerTraceOffset = FVector(3.0f, 0.0f, -4.0f);

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

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* GrabPointMeshRight = nullptr;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* GrabPointMeshLeft = nullptr;

	void SetFistCollisionEnabled(EHand Hand, bool bEnabled);

	void UpdateBodyPositionInVR();

	void DetachItemFromHand(EHand Hand);

	UPROPERTY(EditAnywhere)
		TSubclassOf<AGrabActor> GrabActorToSpawn = nullptr;

	void SpawnActor(EHand Hand);

	UMotionControllerComponent* GetMotionController(EHand Hand);

	void PlayControllerVibration(EHand Hand, bool bIgnoreTimeout, float Scale = 1.0f, EVibrationType Type = EVibrationType::Physics);

	UPROPERTY(EditAnywhere)
		UWidgetInteractionComponent* WidgetInteractionRight = nullptr;

	UPROPERTY(EditAnywhere)
		UWidgetInteractionComponent* WidgetInteractionLeft = nullptr;

	UWidgetInteractionComponent* GetWidgetInteractionComponent(EHand Hand);

	void TriggerStateChanged(EHand Hand, bool bPressed);

	bool IsVREnabled();
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

	UGrabComponent* TraceFromFingerGrabRight;
	UGrabComponent* TraceFromFingerGrabLeft;

	void UpdateNearestItemForTeleport(EHand Hand);

	void OnGrabTeleportAction(EHand Hand);

	void UpdateGrabPoint(EHand Hand);

	void SetNonVRControls();

	bool bIsVREnabled = false;
private:
	FManualTimer VibrationTimerRight = FManualTimer(1.0f);
	FManualTimer VibrationTimerLeft = FManualTimer(1.0f);

};
