// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "../Prop.h"
#include "Door.generated.h"

UCLASS()
class EPSILON_API ADoor : public AProp
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USceneComponent* Main;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Frame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UPhysicsConstraintComponent* Constraint;

};
