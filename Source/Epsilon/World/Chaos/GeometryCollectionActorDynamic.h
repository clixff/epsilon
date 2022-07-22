// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "Field/FieldSystemObjects.h"
#include "Field/FieldSystemComponent.h"
#include "../../Misc/Misc.h"
#include "GeometryCollectionActorDynamic.generated.h"

/**
 * 
 */
UCLASS()
class EPSILON_API AGeometryCollectionActorDynamic : public AGeometryCollectionActor
{
	GENERATED_BODY()
public:
	AGeometryCollectionActorDynamic();
	~AGeometryCollectionActorDynamic();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UUniformVector* UniformVector = nullptr;

	void AddLinearVelocity(FVector Velocity, float Scale = 1.0f);

	void AddLinearVelocityAfterSpawn(FVector Velocity);


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UFieldSystemComponent* Field = nullptr;

	void ApplyDamageForce(float Radius);

	FManualTimer RemovePiecesTimer = FManualTimer(10.0f);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FVector VelocityToAdd = FVector::ZeroVector;
protected:
	virtual void BeginPlay() override;
};
