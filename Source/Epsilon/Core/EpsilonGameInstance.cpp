// Fill out your copyright notice in the Description page of Project Settings.


#include "EpsilonGameInstance.h"

UEpsilonGameInstance* UEpsilonGameInstance::Singleton = nullptr;

UEpsilonGameInstance::UEpsilonGameInstance()
{

}

UEpsilonGameInstance::~UEpsilonGameInstance()
{
	if (UEpsilonGameInstance::Singleton == this)
	{
		UEpsilonGameInstance::Singleton = nullptr;
	}
}

void UEpsilonGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogTemp, Display, TEXT("[UEpsilonGameInstance] ::Init()"));

	UEpsilonGameInstance::Singleton = this;

	GetMapManager()->LoadMainMenu();
}

void UEpsilonGameInstance::Shutdown()
{
	Super::Init();

	UE_LOG(LogTemp, Display, TEXT("[UEpsilonGameInstance] ::Shutdown()"));

	if (UEpsilonGameInstance::Singleton == this)
	{
		UEpsilonGameInstance::Singleton = nullptr;

		if (GameSessionRef && GameSessionRef->IsValidLowLevel())
		{
			GameSessionRef->Shutdown();
		}

		GameSessionRef = nullptr;
	}
}

void UEpsilonGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);

	GetMapManager()->OnWorldChanged(OldWorld, NewWorld);
}

void UEpsilonGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);
	GetMapManager()->OnLevelLoaded(LoadTime, MapName);
}

void UEpsilonGameInstance::StartGameSession(bool bNewGame)
{
	UE_LOG(LogTemp, Display, TEXT("[UEpsilonGameInstance] ::StartGameSession(%d)"), bNewGame);

	auto* GameSession = GetGameSession();

	if (!GameSession)
	{
		return;
	}

	GameSession->Init();

	if (bNewGame)
	{
		GameSession->StartNewGame();
	}
}

UEpsilonGameInstance* UEpsilonGameInstance::Get()
{
	return UEpsilonGameInstance::Singleton;
}

UEpsilonGameSession* UEpsilonGameInstance::GetGameSession()
{
	if (!GameSessionRef)
	{
		if (!GameSessionClass)
		{
			GameSessionClass = UEpsilonGameSession::StaticClass();
		}

		GameSessionRef = NewObject<UEpsilonGameSession>(this, GameSessionClass, TEXT("GameSession"));
	}

	return GameSessionRef;
}

UMapManager* UEpsilonGameInstance::GetMapManager()
{
	if (!MapManagerRef)
	{
		MapManagerRef = NewObject<UMapManager>(this);
	}

	return MapManagerRef;
}
