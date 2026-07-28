// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/CharonInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Character/CharonCharacter.h"
#include "Data/InputFunctionSet.h"


void UCharonInputComponent::AddInputMappings(const UCharonInputConfig* InputConfig,
                                             UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	for(UInputMappingContext* MappingContext : InputConfig->DefaultMappingContexts)
	{
		InputSubsystem->AddMappingContext(MappingContext, 0);
	}

	
}

void UCharonInputComponent::RemoveInputMappings(const UCharonInputConfig* InputConfig,
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	for(UInputMappingContext* MappingContext : InputConfig->DefaultMappingContexts)
	{
		InputSubsystem->RemoveMappingContext(MappingContext);
	}
}

void UCharonInputComponent::BindAbilityActions(const UCharonInputConfig* InputConfig, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for(const FCharonInputAction& CharonInputAction : InputConfig->AbilityInputActions)
	{
		if(CharonInputAction.InputAction && CharonInputAction.InputTag.IsValid())
		{
			BindHandles.Add(BindActionValueLambda(CharonInputAction.InputAction, ETriggerEvent::Triggered,
			                                      [This=this, CharonInputAction](const FInputActionValue& Value)
			                                      {
				                                      This->OnInputActionTriggered.Broadcast(
					                                      Value, CharonInputAction.InputTag, true, true, false);
			                                      }).GetHandle());


			BindHandles.Add(BindActionValueLambda(CharonInputAction.InputAction, ETriggerEvent::Completed,
			                                      [This=this, CharonInputAction](const FInputActionValue& Value)
			                                      {
				                                      This->OnInputActionTriggered.Broadcast(
					                                      Value, CharonInputAction.InputTag, true, false, false);
			                                      }).GetHandle());
			
		}
	}
	
}

// //template <class UserClass, typename FuncType>
// void UCharonInputComponent::BindNativeFunctions(const UCharonInputConfig* InputConfig, ACharacter* Initiator, AInputFunctionSet* InputFunctions, TArray<uint32>& BindHandles)
// {
// 	check(InputConfig);
// 	
// 	for(auto& Element : InputFunctions->InputFunctionDelegateMap)
// 	{
// 		if(const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(Element.Key, false))
// 		{
// 			//const ETriggerEvent TriggerEvent = Element.Value.TriggerEvent;
// 			TArray<ETriggerEvent> TriggerEvents = Element.Value.TriggerEvents;
//
// 			for(ETriggerEvent TriggerEvent : TriggerEvents)
// 			{
// 				ACharonCharacter* CharonCharacter =  Cast<ACharonCharacter>(Initiator);
// 				BindHandles.Add( BindActionValueLambda(InputAction, TriggerEvent, [InputFunctions, CharonCharacter, Element](const FInputActionValue& Value)
// 				{
// 					CharonCharacter->RequestExecuteInputFunction(Value, InputFunctions, Element.Key, Element.Value.bNeedServerRPC );
// 				}).GetHandle() );
// 			}
// 		}
// 	}
// 	
// 	
// }

void UCharonInputComponent::BindNativeFunctions(const UCharonInputConfig* InputConfig, UInputAssistComponent* InputAssistComp, AInputFunctionSet* InputFunctions, TArray<uint32>& BindHandles)
{
	check(InputConfig);
	
	for(auto& Element : InputFunctions->InputFunctionDelegateMap)
	{
		if(const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(Element.Key, false))
		{
			//const ETriggerEvent TriggerEvent = Element.Value.TriggerEvent;
			TArray<ETriggerEvent> TriggerEvents = Element.Value.TriggerEvents;

			for(ETriggerEvent TriggerEvent : TriggerEvents)
			{
				// BindHandles.Add( BindActionValueLambda(InputAction, TriggerEvent, [InputFunctions, InputAssistComp, Element](const FInputActionValue& Value)
				// {
				// 	InputAssistComp->RequestExecuteInputFunction(Value, InputFunctions, Element.Key, Element.Value.bNeedServerRPC );
				// }).GetHandle() );
				BindHandles.Add( BindActionValueLambda(InputAction, TriggerEvent, [This=this, Element](const FInputActionValue& Value)
				{
					This->OnInputActionTriggered.Broadcast(Value, Element.Key, false, false, Element.Value.bNeedServerRPC);
				}).GetHandle() );
			}
		}
	}
	
	
}


void UCharonInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
	for(const uint32 Handle : BindHandles)
	{
		RemoveBindingByHandle(Handle);	
	}
	
}

