// Fill out your copyright notice in the Description page of Project Settings.


#include "MapManager.h"
#include "Kismet/GameplayStatics.h"

#define MAIN_MENU_MAP TEXT("Menu_Map")
#define MAIN_MAP TEXT("Main_Map")

UMapManager::UMapManager()
{

}

UMapManager::~UMapManager()
{

}

void UMapManager::LoadLevel(FString NewLevelName)
{
	UE_LOG(LogTemp, Display, TEXT("[UMapManager] LoadLevel \"%s\""), *NewLevelName);

	UGameplayStatics::OpenLevel(GetWorld(), FName(*NewLevelName), true);
	bLevelLoaded = false;
}

void UMapManager::LoadMainMenu()
{
	LoadLevel(MAIN_MENU_MAP);
}

void UMapManager::LoadMainMap()
{
	LoadLevel(MAIN_MAP);
}

void UMapManager::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	FString NewWorldName = NewWorld ? NewWorld->GetName() : TEXT("NULL");
	LevelName = NewWorldName;

	UE_LOG(LogTemp, Display, TEXT("[UMapManager] OnWorldChanged to \"%s\""), *NewWorldName);

	World = NewWorld;
}

void UMapManager::OnLevelLoaded(const float LoadTime, const FString& MapName)
{
	bLevelLoaded = true;

	UE_LOG(LogTemp, Display, TEXT("[UMapManager] Level \"%s\" loaded"), *LevelName);
	if (this->LevelName == MAIN_MENU_MAP)
	{

	}
	else if (this->LevelName == MAIN_MAP)
	{
		
	}
}
