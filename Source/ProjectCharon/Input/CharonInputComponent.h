// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonInputConfig.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Data/InputFunctionSet.h"
//#include "Data/InputFunctionSet.h"
#include "CharonInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCHARON_API UCharonInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public :
	UCharonInputComponent(){};

	void AddInputMappings(const UCharonInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UCharonInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;

	template<class UserClass, typename FuncType>
	void BindNativeAction(const UCharonInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);

	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UCharonInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	//template<class UserClass, typename FuncType>
	void BindNativeFunctions(const UCharonInputConfig* InputConfig, ACharacter* Initiator, AInputFunctionSet* InputFunctions, TArray<uint32>& BindHandles);
	
	void RemoveBinds(TArray<uint32>& BindHandles);
};

template <class UserClass, typename FuncType>
void UCharonInputComponent::BindNativeAction(const UCharonInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if(const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(InputAction, TriggerEvent, Object, Func);
	}
}

template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UCharonInputComponent::BindAbilityActions(const UCharonInputConfig* InputConfig, UserClass* Object,
	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for(const FCharonInputAction& CharonInputAction : InputConfig->AbilityInputActions)
	{
		if(CharonInputAction.InputAction && CharonInputAction.InputTag.IsValid())
		{
			if(PressedFunc)
			{
				BindHandles.Add( BindAction(CharonInputAction.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, CharonInputAction.InputTag).GetHandle() );
			}

			if(ReleasedFunc)
			{
				BindHandles.Add( BindAction(CharonInputAction.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, CharonInputAction.InputTag).GetHandle() );
			}
		}
	}
}




