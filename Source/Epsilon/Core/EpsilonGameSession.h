// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundBase.h"
#include "EpsilonGameSession.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class EPSILON_API UEpsilonGameSession : public UObject
{
	GENERATED_BODY()
public:
	UEpsilonGameSession();
	~UEpsilonGameSession();

	void Init();
	void Shutdown();

	void StartNewGame();
public:
	static UEpsilonGameSession* Singleton;
	static UEpsilonGameSession* Get();
public:

	UPROPERTY(EditDefaultsOnly)
		USoundBase* PhysicsHitSound = nullptr;
};
