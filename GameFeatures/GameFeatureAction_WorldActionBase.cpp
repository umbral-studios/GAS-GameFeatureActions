// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatureAction_WorldActionBase.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Engine/World.h" // for FWorldDelegates::OnStartGameInstance
#include "Engine/Engine.h" // for FWorldContext

void UGameFeatureAction_WorldActionBase::OnGameFeatureActivating()
{
	GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UGameFeatureAction_WorldActionBase::HandleGameInstanceStart);

	// Add to any worlds with associated game instances that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		AddToWorld(WorldContext);
	}
}

void UGameFeatureAction_WorldActionBase::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartHandle);
}

void UGameFeatureAction_WorldActionBase::HandleGameInstanceStart(UGameInstance* GameInstance)
{
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		AddToWorld(*WorldContext);
	}
}
