// Copyright Epic Games, Inc. All Rights Reserved.

#include "<PROJECT_NAME>AbilityTargetActor.h"
#include "<PROJECT_NAME>.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/BlueprintGeneratedClass.h"

void A<PROJECT_NAME>AbilityTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	
	RecieveStartTargeting(Ability);
}

bool A<PROJECT_NAME>AbilityTargetActor::IsConfirmTargetingAllowed()
{
	return K2_IsConfirmTargetingAllowed();
}

void A<PROJECT_NAME>AbilityTargetActor::ConfirmTargetingAndContinue()
{
	if (SourceActor && ensureAlways(ShouldProduceTargetData()))
	{
		auto ImplementedInBlueprint = [](const UFunction* Func) -> bool
		{
			return Func && ensure(Func->GetOuter())
				&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
		};
		UE_CLOG(
			!ImplementedInBlueprint(GetClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(A<PROJECT_NAME>AbilityTargetActor, MakeTargetData))),
			Log<PROJECT_NAME>,
			Error,
			TEXT("`%s` does not implement `MakeTargetData`. Invalid target data will be returned."),
			*GetClass()->GetName()
		);

		FGameplayAbilityTargetDataHandle Handle = MakeTargetData();
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

bool A<PROJECT_NAME>AbilityTargetActor::K2_IsConfirmTargetingAllowed_Implementation()
{
	return Super::IsConfirmTargetingAllowed();
}