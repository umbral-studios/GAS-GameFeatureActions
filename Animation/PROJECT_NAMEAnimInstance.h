// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Animation/AnimInstance.h"
#include "GameplayEffectTypes.h" // for FGameplayTagBlueprintPropertyMap
#include "<PROJECT_NAME>AnimInstance.generated.h"

class UAbilitySystemComponent;

UCLASS()
class U<PROJECT_NAME>AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

public:
	//~ Begin UAnimInstance interface
	virtual void NativeInitializeAnimation() override;
	//~ End UAnimInstance interface

	void InitializeWithAbilitySystem(UAbilitySystemComponent* AbilityComponent);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Initialize with Ability System")
	void ReceiveAbilitySystem(UAbilitySystemComponent* AbilitySystem);
};
