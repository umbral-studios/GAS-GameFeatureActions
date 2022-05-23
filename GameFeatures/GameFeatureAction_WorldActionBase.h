// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "GameFeatureAction_WorldActionBase.generated.h"

class UGameInstance;
struct FWorldContext;

/**
 * Base class for GameFeatureActions that wish to do something world specific.
 */
UCLASS(Abstract)
class UGameFeatureAction_WorldActionBase : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureActivating() override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	//~ End UGameFeatureAction interface

private:
	void HandleGameInstanceStart(UGameInstance* GameInstance);
	virtual void AddToWorld(const FWorldContext& WorldContext) PURE_VIRTUAL(UGameFeatureAction_WorldActionBase::AddToWorld,);

private:
	FDelegateHandle GameInstanceStartHandle;
};
