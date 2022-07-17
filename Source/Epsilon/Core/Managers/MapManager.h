// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MapManager.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class EPSILON_API UMapManager : public UObject
{
	GENERATED_BODY()
public:
	UMapManager();

	~UMapManager();

	UPROPERTY(Export)
		UWorld* World = nullptr;

	UPROPERTY()
		bool bLevelLoaded = false;

	void LoadLevel(FString NewLevelName);

	void LoadMainMenu();

	void LoadMainMap();

	void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld);

	void OnLevelLoaded(const float LoadTime, const FString& MapName);

	UPROPERTY()
		FString LevelName = TEXT("NULL");
};
