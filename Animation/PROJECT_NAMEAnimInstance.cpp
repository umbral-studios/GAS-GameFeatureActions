// Copyright Epic Games, Inc. All Rights Reserved.

#include "<PROJECT_NAME>AnimInstance.h"
#include "AbilitySystemComponent.h"

void U<PROJECT_NAME>AnimInstance::NativeInitializeAnimation()
{
	if (AActor* MyActor = GetOwningActor())
	{
		UAbilitySystemComponent* AbilityComponent = MyActor->FindComponentByClass<UAbilitySystemComponent>();
		if (AbilityComponent)
		{
			InitializeWithAbilitySystem(AbilityComponent);
		}
	}

	Super::NativeInitializeAnimation();
}

void U<PROJECT_NAME>AnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* AbilityComponent)
{
	GameplayTagPropertyMap.Initialize(this, AbilityComponent);
	ReceiveAbilitySystem(AbilityComponent);
}