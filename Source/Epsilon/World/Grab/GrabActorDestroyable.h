// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemObjects.h"
#include "GrabActorDestroyable.generated.h"

/**
 * 
 */
UCLASS()
class EPSILON_API AGrabActorDestroyable : public AGrabActor
{
	GENERATED_BODY()
public:
	AGrabActorDestroyable();

	~AGrabActorDestroyable();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UGeometryCollection* GeometryCollection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float DamageRadius = 25.0f;

	/** In seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RemovePiecesTime = 15.0f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void DamageObject() override;
};
