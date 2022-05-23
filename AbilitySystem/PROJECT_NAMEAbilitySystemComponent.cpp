// Copyright Epic Games, Inc. All Rights Reserved.

#include "<PROJECT_NAME>AbilitySystemComponent.h"
#include "Animation/<PROJECT_NAME>AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystem/<PROJECT_NAME>AbilityAttributeSet.h"

void U<PROJECT_NAME>AbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (AbilityActorInfo)
	{
		if (AbilityActorInfo->AnimInstance == nullptr)
		{
			AbilityActorInfo->AnimInstance = AbilityActorInfo->GetAnimInstance();
		}

		if (U<PROJECT_NAME>AnimInstance* TopAnimInststance = Cast<U<PROJECT_NAME>AnimInstance>(AbilityActorInfo->AnimInstance))
		{
			TopAnimInststance->InitializeWithAbilitySystem(this);
		}

		if (AbilityActorInfo->SkeletalMeshComponent.IsValid())
		{
			const USkeletalMeshComponent* SkeletalMeshComponent = AbilityActorInfo->SkeletalMeshComponent.Get();

			const TArray<UAnimInstance*>& AnimLayers = SkeletalMeshComponent->GetLinkedAnimInstances();
			for (UAnimInstance* AnimLayer : AnimLayers)
			{
				if (U<PROJECT_NAME>AnimInstance* <PROJECT_NAME>AnimLayer = Cast<U<PROJECT_NAME>AnimInstance>(AnimLayer))
				{
					<PROJECT_NAME>AnimLayer->InitializeWithAbilitySystem(this);
				}
			}
		}

		if (UGameInstance* GameInstance = InOwnerActor->GetGameInstance())
		{
			// Sign up for possess/unpossess events so that we can update the cached AbilityActorInfo accordingly
			GameInstance->GetOnPawnControllerChanged().AddDynamic(this, &U<PROJECT_NAME>AbilitySystemComponent::OnPawnControllerChanged);
		}
	}

	GrantDefaultAbilitiesAndAttributes();
}

void U<PROJECT_NAME>AbilitySystemComponent::BeginDestroy()
{
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor.IsValid())
	{
		if (UGameInstance* GameInstance = AbilityActorInfo->OwnerActor->GetGameInstance())
		{
			GameInstance->GetOnPawnControllerChanged().RemoveAll(this);
		}
	}

	Super::BeginDestroy();
}

FGameplayAbilitySpecHandle U<PROJECT_NAME>AbilitySystemComponent::GrantAbilityOfType(TSubclassOf<UGameplayAbility> AbilityType, bool bRemoveAfterActivation)
{
	FGameplayAbilitySpecHandle AbilityHandle;
	if (AbilityType)
	{
		FGameplayAbilitySpec AbilitySpec(AbilityType);
		AbilitySpec.RemoveAfterActivation = bRemoveAfterActivation;

		AbilityHandle = GiveAbility(AbilitySpec);
	}
	return AbilityHandle;
}

void U<PROJECT_NAME>AbilitySystemComponent::GrantDefaultAbilitiesAndAttributes()
{
	// Reset/Remove abilities if we had already added them
	{
		for (UAttributeSet* AttribSetInstance : AddedAttributes)
		{
			GetSpawnedAttributes_Mutable().Remove(AttribSetInstance);
		}

		for (FGameplayAbilitySpecHandle AbilityHandle : DefaultAbilityHandles)
		{
			SetRemoveAbilityOnEnd(AbilityHandle);
		}

		AddedAttributes.Empty(DefaultAttributes.Num());
		DefaultAbilityHandles.Empty(DefaultAbilities.Num());
	}

	// Default abilities
	{
		DefaultAbilityHandles.Reserve(DefaultAbilities.Num());
		for (const TSubclassOf<UGameplayAbility>& Ability : DefaultAbilities)
		{
			if (*Ability)
			{
				DefaultAbilityHandles.Add(GiveAbility(FGameplayAbilitySpec(Ability)));
			}
		}
	}

	// Default attributes
	{
		for (const F<PROJECT_NAME>AttributeApplication& Attributes : DefaultAttributes)
		{
			if (Attributes.AttributeSetType)
			{
				UAttributeSet* NewAttribSet = NewObject<UAttributeSet>(this, Attributes.AttributeSetType);
				if (Attributes.InitializationData)
				{
					NewAttribSet->InitFromMetaDataTable(Attributes.InitializationData);
				}
				AddedAttributes.Add(NewAttribSet);
				AddAttributeSetSubobject(NewAttribSet);
			}
		}
	}
}

void U<PROJECT_NAME>AbilitySystemComponent::OnPawnControllerChanged(APawn* Pawn, AController* NewController)
{
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor == Pawn && AbilityActorInfo->PlayerController != NewController)
	{
		// Reinit the cached ability actor info (specifically the player controller)
		AbilityActorInfo->InitFromActor(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get(), this);
	}
}
