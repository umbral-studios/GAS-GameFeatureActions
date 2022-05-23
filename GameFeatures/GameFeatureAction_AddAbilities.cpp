// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatureAction_AddAbilities.h"
#include "<PROJECT_NAME>/<PROJECT_NAME>.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Engine/AssetManager.h"
#include "<PROJECT_NAME>/AbilitySystem/<PROJECT_NAME>AbilitySystemComponent.h"
#include "<PROJECT_NAME>/AbilitySystem/<PROJECT_NAME>AbilityAttributeSet.h"
#include "<PROJECT_NAME>/AbilitySystem/AbilityInputBindingComponent.h"

#define LOCTEXT_NAMESPACE "<PROJECT_NAME>Features"

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddAbilities

void UGameFeatureAction_AddAbilities::OnGameFeatureActivating()
{
	if (!ensureAlways(ActiveExtensions.IsEmpty()) ||
		!ensureAlways(ComponentRequests.IsEmpty()))
	{
		Reset();
	}
	Super::OnGameFeatureActivating();
}

void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	Reset();
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddAbilities::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsValid())
	{
		auto AddBundleAsset = [&AssetBundleData](const FSoftObjectPath& SoftObjectPath)
		{
			AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, SoftObjectPath);
			AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, SoftObjectPath);
		};

		for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
		{
			for (const F<PROJECT_NAME>AbilityMapping& Ability : Entry.GrantedAbilities)
			{
				AddBundleAsset(Ability.AbilityType.ToSoftObjectPath());
				if (!Ability.InputAction.IsNull())
				{
					AddBundleAsset(Ability.InputAction.ToSoftObjectPath());
				}
			}

			for (const F<PROJECT_NAME>AttributesMapping& Attributes : Entry.GrantedAttributes)
			{
				AddBundleAsset(Attributes.AttributeSetType.ToSoftObjectPath());
				if (!Attributes.InitializationData.IsNull())
				{
					AddBundleAsset(Attributes.InitializationData.ToSoftObjectPath());
				}
			}
		}
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddAbilities::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList"), FText::AsNumber(EntryIndex)));
		}

		if (Entry.GrantedAbilities.IsEmpty() && Entry.GrantedAttributes.IsEmpty())
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNoAddOns", "Empty GrantedAbilities and GrantedAttributes at index {0} in AbilitiesList"), FText::AsNumber(EntryIndex)));
		}

		int32 AbilityIndex = 0;
		for (const F<PROJECT_NAME>AbilityMapping& Ability : Entry.GrantedAbilities)
		{
			if (Ability.AbilityType.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullAbility", "Null AbilityType at index {0} in AbilitiesList[{1}].GrantedAbilities"), FText::AsNumber(AbilityIndex), FText::AsNumber(EntryIndex)));
			}
			++AbilityIndex;
		}

		int32 AttributesIndex = 0;
		for (const F<PROJECT_NAME>AttributesMapping& Attributes : Entry.GrantedAttributes)
		{
			if (Attributes.AttributeSetType.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullAttributeSet", "Null AttributeSetType at index {0} in AbilitiesList[{1}].GrantedAttributes"), FText::AsNumber(AttributesIndex), FText::AsNumber(EntryIndex)));
			}
			++AttributesIndex;
		}

		++EntryIndex;
	}

	return Result;

	return EDataValidationResult::NotValidated;
}
#endif

void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{			
			int32 EntryIndex = 0;
			for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
						this, &UGameFeatureAction_AddAbilities::HandleActorExtension, EntryIndex);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);

					ComponentRequests.Add(ExtensionRequestHandle);
					EntryIndex++;
				}
			}
		}
	}
}

void UGameFeatureAction_AddAbilities::Reset()
{
	while (!ActiveExtensions.IsEmpty())
	{
		auto ExtensionIt = ActiveExtensions.CreateIterator();
		RemoveActorAbilities(ExtensionIt->Key);
	}

	ComponentRequests.Empty();
}

void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex)
{
	if (AbilitiesList.IsValidIndex(EntryIndex))
	{
		const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
		{
			RemoveActorAbilities(Actor);
		}
		else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
		{
			AddActorAbilities(Actor, Entry);
		}
	}
}

void UGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry)
{
	UE_LOG(Log<PROJECT_NAME>, Error, TEXT("Added ability"), *Actor->GetPathName());
	if (UAbilitySystemComponent* AbilitySystemComponent = FindOrAddComponentForActor<UAbilitySystemComponent>(Actor, AbilitiesEntry))
	{
		FActorExtensions AddedExtensions;
		AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num());
		AddedExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAttributes.Num());

		for (const F<PROJECT_NAME>AbilityMapping& Ability : AbilitiesEntry.GrantedAbilities)
		{
			if (!Ability.AbilityType.IsNull())
			{
				FGameplayAbilitySpec NewAbilitySpec(Ability.AbilityType.LoadSynchronous());
				FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

				if (!Ability.InputAction.IsNull())
				{
					UAbilityInputBindingComponent* InputComponent = FindOrAddComponentForActor<UAbilityInputBindingComponent>(Actor, AbilitiesEntry);
					if (InputComponent)
					{
						InputComponent->SetInputBinding(Ability.InputAction.LoadSynchronous(), AbilityHandle);
					}
					else
					{
						 UE_LOG(Log<PROJECT_NAME>, Error, TEXT("Failed to find/add an ability input binding component to '%s' -- are you sure it's a pawn class?"), *Actor->GetPathName());
					}
				}

				AddedExtensions.Abilities.Add(AbilityHandle);
			}
		}

		for (const F<PROJECT_NAME>AttributesMapping& Attributes : AbilitiesEntry.GrantedAttributes)
		{
			if (!Attributes.AttributeSetType.IsNull())
			{
				TSubclassOf<U<PROJECT_NAME>AbilityAttributeSet> SetType = Attributes.AttributeSetType.LoadSynchronous();
				if (SetType)
				{
					UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent, SetType);
					if (!Attributes.InitializationData.IsNull())
					{
						UDataTable* InitData = Attributes.InitializationData.LoadSynchronous();
						if (InitData)
						{
							NewSet->InitFromMetaDataTable(InitData);
						}
					}

					AddedExtensions.Attributes.Add(NewSet);
					AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
				}
			}
		}

		ActiveExtensions.Add(Actor, AddedExtensions);
	}
	else
	{
		UE_LOG(Log<PROJECT_NAME>, Error, TEXT("Failed to find/add an ability component to '%s'. Abilities will not be granted."), *Actor->GetPathName());
	}
}

void UGameFeatureAction_AddAbilities::RemoveActorAbilities(AActor* Actor)
{
	if (FActorExtensions* ActorExtensions = ActiveExtensions.Find(Actor))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			for (UAttributeSet* AttribSetInstance : ActorExtensions->Attributes)
			{
				AbilitySystemComponent->GetSpawnedAttributes_Mutable().Remove(AttribSetInstance);
			}

			UAbilityInputBindingComponent* InputComponent = Actor->FindComponentByClass<UAbilityInputBindingComponent>();
			for (FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->Abilities)
			{
				if (InputComponent)
				{
					InputComponent->ClearInputBinding(AbilityHandle);
				}
				AbilitySystemComponent->SetRemoveAbilityOnEnd(AbilityHandle);
			}
		}

		ActiveExtensions.Remove(Actor);
	}
}

UActorComponent* UGameFeatureAction_AddAbilities::FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry)
{
	UActorComponent* Component = Actor->FindComponentByClass(ComponentType);
	
	bool bMakeComponentRequest = (Component == nullptr);
	if (Component)
	{
		// Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
		// `Native` is what `CreationMethod` defaults to for dynamically added components.
		if (Component->CreationMethod == EComponentCreationMethod::Native)
		{
			// Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
			// If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
			UObject* ComponentArchetype = Component->GetArchetype();
			bMakeComponentRequest = ComponentArchetype->HasAnyFlags(RF_ClassDefaultObject);
		}
	}

	if (bMakeComponentRequest)
	{
		UWorld* World = Actor->GetWorld();
		UGameInstance* GameInstance = World->GetGameInstance();

		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentMan->AddComponentRequest(AbilitiesEntry.ActorClass, ComponentType);
			ComponentRequests.Add(RequestHandle);
		}

		if (!Component)
		{
			Component = Actor->FindComponentByClass(ComponentType);
			ensureAlways(Component);
		}
	}

	return Component;
}

#undef LOCTEXT_NAMESPACE
