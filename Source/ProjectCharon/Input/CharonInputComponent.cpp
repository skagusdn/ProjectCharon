// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/CharonInputComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Character/CharonCharacter.h"


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

//template <class UserClass, typename FuncType>
void UCharonInputComponent::BindNativeFunctions(const UCharonInputConfig* InputConfig, ACharacter* Initiator, AInputFunctionSet* InputFunctions, TArray<uint32>& BindHandles)
{
	check(InputConfig);
	
	for(auto& Element : InputFunctions->InputFunctionDelegateMap)
	{
		if(const UInputAction* InputAction = InputConfig->FindNativeInputActionForTag(Element.Key, false))
		{
			const ETriggerEvent TriggerEvent = Element.Value.TriggerEvent;

			ACharonCharacter* CharonCharacter =  Cast<ACharonCharacter>(Initiator);
			BindHandles.Add( BindActionValueLambda(InputAction, TriggerEvent, [InputFunctions, CharonCharacter, Element](const FInputActionValue& Value)
			{
				CharonCharacter->RequestExecuteInputFunction(Value, InputFunctions, Element.Key, Element.Value.bNeedServerRPC );
			}).GetHandle() );
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
