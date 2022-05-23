// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatureAction_AddInputContextMapping.h"
#include "<PROJECT_NAME>.h"
#include "Engine/AssetManager.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Character/<PROJECT_NAME>PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

#define LOCTEXT_NAMESPACE "<PROJECT_NAME>Features"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddInputContextMapping

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureActivating()
{
	if (!ensure(ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ControllersAddedTo.IsEmpty()))
	{
		Reset();
	}
	Super::OnGameFeatureActivating();
}

void UGameFeatureAction_AddInputContextMapping::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	Reset();
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddInputContextMapping::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsValid())
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, InputMapping.ToSoftObjectPath());
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, InputMapping.ToSoftObjectPath());
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputContextMapping::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	if (InputMapping.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		ValidationErrors.Add(LOCTEXT("NullInputMapping", "Null InputMapping."));
	}

	return Result;
}
#endif

void UGameFeatureAction_AddInputContextMapping::AddToWorld(const FWorldContext& WorldContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			if (!InputMapping.IsNull())
			{
				UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
					this, &UGameFeatureAction_AddInputContextMapping::HandleControllerExtension);
				TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(APlayerController::StaticClass(), AddAbilitiesDelegate);

				ExtensionRequestHandles.Add(ExtensionRequestHandle);
			}
		}
	}
}

void UGameFeatureAction_AddInputContextMapping::Reset()
{
	ExtensionRequestHandles.Empty();

	while (!ControllersAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> ControllerPtr = ControllersAddedTo.Top();
		if (ControllerPtr.IsValid())
		{
			RemoveInputMapping(ControllerPtr.Get());
		}
		else
		{
			ControllersAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputContextMapping::HandleControllerExtension(AActor* Actor, FName EventName)
{
	APlayerController* AsController = CastChecked<APlayerController>(Actor);

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveInputMapping(AsController);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		AddInputMappingForPlayer(AsController->GetLocalPlayer());
	}
}

void UGameFeatureAction_AddInputContextMapping::AddInputMappingForPlayer(UPlayer* Player)
{
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!InputMapping.IsNull())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), Priority);
			}
		}
		else
		{
			UE_LOG(Log<PROJECT_NAME>, Error, TEXT("Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."));
		}
	}
}

void UGameFeatureAction_AddInputContextMapping::RemoveInputMapping(APlayerController* PlayerController)
{
	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSystem->RemoveMappingContext(InputMapping.Get());
		}
	}

	ControllersAddedTo.Remove(PlayerController);
}

#undef LOCTEXT_NAMESPACE