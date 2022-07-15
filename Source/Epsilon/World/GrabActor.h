// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "../Components/GrabComponent.h"
#include "../Misc/Misc.h"
#include "GrabActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class EPSILON_API AGrabActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrabActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY(EditAnywhere)
		UGrabComponent* GrabComponent = nullptr;

	UPROPERTY(EditAnywhere)
		bool bShouldSimulatePhysics = true;

	UPROPERTY(VisibleAnywhere)
		bool bGrabbing = false;

	void OnGrab(EHand Hand);

	void OnUnGrab();

	bool bFlyingToController = false;

	ECollisionEnabled::Type CollisionType = ECollisionEnabled::QueryAndPhysics;

};
