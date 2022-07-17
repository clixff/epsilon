// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EpsilonGameSession.h"
#include "Managers/MapManager.h"
#include "EpsilonGameInstance.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class EPSILON_API UEpsilonGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UEpsilonGameInstance();
	~UEpsilonGameInstance();

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init() override;

	/** virtual function to allow custom GameInstances an opportunity to do cleanup when shutting down */
	virtual void Shutdown() override;

	/** Callback from the world context when the world changes */
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;

	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	UFUNCTION(BlueprintCallable)
		void StartGameSession(bool bNewGame = true);
public:
	static UEpsilonGameInstance* Singleton;

	/** Get Singleton */
	UFUNCTION(BlueprintCallable)
		static UEpsilonGameInstance* Get();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UEpsilonGameSession> GameSessionClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Export)
		UEpsilonGameSession* GameSessionRef;

	UEpsilonGameSession* GetGameSession();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Export)
		UMapManager* MapManagerRef;

	UMapManager* GetMapManager();
};
