// Fill out your copyright notice in the Description page of Project Settings.


#include "input/CharonInputConfig.h"

const UInputAction* UCharonInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag,
	bool bLogNotFound) const
{
	for (const FCharonInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}


const UInputAction* UCharonInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
                                                                     bool bLogNotFound) const
{
	for (const FCharonInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag [%s] on InputConfig [%s]."), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
