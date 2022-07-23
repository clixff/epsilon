// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ObjectMacros.h"
#include "Misc.generated.h"

UENUM()
enum class EHand : uint8
{
	Left,
	Right
};

UENUM()
enum class EVibrationType : uint8
{
	Physics,
	Grab
};


USTRUCT(BlueprintType)
struct FManualTimer
{
	GENERATED_BODY()
public:
	FManualTimer()
	{

	}

	FManualTimer(float MaxSeconds)
	{
		this->Max = MaxSeconds;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float Current = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Max = 10.0f;

	void Add(float DeltaTime)
	{
		this->Current += DeltaTime;
		if (this->Current > this->Max)
		{
			this->Current = this->Max;
		}
	}

	bool IsEnded()
	{
		return this->Current >= this->Max;
	}

	void Reset()
	{
		this->Current = 0.0f;
	}
};