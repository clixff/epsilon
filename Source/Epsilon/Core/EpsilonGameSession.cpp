// Fill out your copyright notice in the Description page of Project Settings.


#include "EpsilonGameSession.h"
#include "EpsilonGameInstance.h"

UEpsilonGameSession* UEpsilonGameSession::Singleton = nullptr;

UEpsilonGameSession::UEpsilonGameSession()
{

}

UEpsilonGameSession::~UEpsilonGameSession()
{

}

void UEpsilonGameSession::Init()
{
	UE_LOG(LogTemp, Display, TEXT("[UEpsilonGameSession] ::Init()"));

	UEpsilonGameSession::Singleton = this;
}

void UEpsilonGameSession::Shutdown()
{
	if (UEpsilonGameSession::Singleton == this)
	{
		UEpsilonGameSession::Singleton = nullptr;
	}
}

void UEpsilonGameSession::StartNewGame()
{
	UE_LOG(LogTemp, Display, TEXT("[UEpsilonGameSession] ::StartNewGame()"));

	auto* MapManager = UEpsilonGameInstance::Get()->GetMapManager();
	MapManager->LoadMainMap();
}

UEpsilonGameSession* UEpsilonGameSession::Get()
{
	return UEpsilonGameSession::Singleton;
}
