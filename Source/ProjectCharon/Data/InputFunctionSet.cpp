// Fill out your copyright notice in the Description page of Project Settings.


#include "InputFunctionSet.h"


void UInputFunctionSet::AddEvent(FGameplayTag Tag, FInputActionDelegate Delegate, ETriggerEvent TriggerEvent)
{
	InputFunctionDelegateMap.Add(Tag, {Delegate, TriggerEvent});
}
