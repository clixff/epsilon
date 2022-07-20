// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "../Misc/Misc.h"
#include "GrabComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Grab Component", BlueprintSpawnableComponent))
class EPSILON_API UGrabComponent : public UBoxComponent
{
	GENERATED_BODY()
public:
	UGrabComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere)
		bool bGrabbing = false;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnGrab(EHand Hand);

	void OnUnGrab(EHand Hand);

	UPROPERTY()
		UPrimitiveComponent* ControllerToFlyTo = nullptr;

	void FlyToController(UPrimitiveComponent* Controller, EHand Hand);

	void GetTransformForAttach(EHand Hand, FVector& Location, FRotator& Rotation);

	EHand HandToAttach = EHand::Right;
private:
	/** Flying to hand */

	FManualTimer FlyTimer = FManualTimer(1.0f);

	FVector FlyStartLocation;
	FRotator FlyStartRotation;

	void FlyToControllerTick(float DeltaTime);
};
