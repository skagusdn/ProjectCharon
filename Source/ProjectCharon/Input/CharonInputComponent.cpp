// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/CharonInputComponent.h"

#include "EnhancedInputSubsystems.h"


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
			FInputActionDelegate Delegate = Element.Value.Delegate;
			const ETriggerEvent TriggerEvent = Element.Value.TriggerEvent;
			
			BindHandles.Add( BindActionValueLambda(InputAction, TriggerEvent, [Delegate, Initiator](const FInputActionValue& Value) -> void
			{
				if(Delegate.IsBound())
				{
					Delegate.Execute(Value, Initiator);
				}
			} ).GetHandle() );
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
