// Fill out your copyright notice in the Description page of Project Settings.


#include "InputFunctionSet.h"


AInputFunctionSet::AInputFunctionSet()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AInputFunctionSet::AddEvent(FGameplayTag Tag, FInputActionDelegate Delegate, ETriggerEvent TriggerEvent)
{
	InputFunctionDelegateMap.Add(Tag, {Delegate, TriggerEvent});
}

