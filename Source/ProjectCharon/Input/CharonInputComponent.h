// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharonInputConfig.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
// #include "Character/InputAssistComponent.h"
// #include "Data/InputFunctionSet.h"
// #include "GameFramework/Character.h"
//#include "Data/InputFunctionSet.h"
#include "CharonInputComponent.generated.h"

class UInputAssistComponent;
class AInputFunctionSet;


DECLARE_MULTICAST_DELEGATE_FiveParams(FOnInputActionTriggered, const FInputActionValue& ActionValue, FGameplayTag InputTag, bool bIsAbilityAction, bool bIsButtonPressed, bool bNeedServerRPC);

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
	
	//template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	//void BindAbilityActions(const UCharonInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);
	
	void BindAbilityActions(const UCharonInputConfig* InputConfig, TArray<uint32>& BindHandles);
	void BindNativeFunctions(const UCharonInputConfig* InputConfig, UInputAssistComponent* InputAssistComp, AInputFunctionSet* InputFunctions, TArray<uint32>& BindHandles);
	void RemoveBinds(TArray<uint32>& BindHandles);
	
	template<class UserClass, typename CallbackFuncType>
	void RegisterInputReceiver(UserClass* Object, CallbackFuncType CallbackFunc);
	
	
	FOnInputActionTriggered OnInputActionTriggered;
};


// template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
// void UCharonInputComponent::BindAbilityActions(const UCharonInputConfig* InputConfig, UserClass* Object,
// 	PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
// {
// 	check(InputConfig);
//
// 	for(const FCharonInputAction& CharonInputAction : InputConfig->AbilityInputActions)
// 	{
// 		if(CharonInputAction.InputAction && CharonInputAction.InputTag.IsValid())
// 		{
// 			// if(PressedFunc)
// 			// {
// 			// 	BindHandles.Add( BindAction(CharonInputAction.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, CharonInputAction.InputTag).GetHandle() );
// 			// }
// 			//
// 			// if(ReleasedFunc)
// 			// {
// 			// 	BindHandles.Add( BindAction(CharonInputAction.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, CharonInputAction.InputTag).GetHandle() );
// 			// }
// 			if(PressedFunc)
// 			{
// 				BindHandles.Add( BindActionValueLambda(CharonInputAction.InputAction, ETriggerEvent::Triggered, [This=this, CharonInputAction](const FInputActionValue& Value)
// 				{
// 					This->OnInputActionTriggered.Broadcast(Value, CharonInputAction.InputTag, true, true, false);
// 				}).GetHandle() );
// 				
// 			}
//
// 			if(ReleasedFunc)
// 			{
// 				BindHandles.Add( BindActionValueLambda(CharonInputAction.InputAction, ETriggerEvent::Completed, [This=this, CharonInputAction](const FInputActionValue& Value)
// 				{
// 					This->OnInputActionTriggered.Broadcast(Value, CharonInputAction.InputTag, true, false, false);
// 				}).GetHandle() );
// 			}
// 		}
// 	}
// }

template <class UserClass, typename CallbackFuncType>
void UCharonInputComponent::RegisterInputReceiver(UserClass* Object, CallbackFuncType CallbackFunc)
{
	OnInputActionTriggered.AddUObject(Object, CallbackFunc);
}




