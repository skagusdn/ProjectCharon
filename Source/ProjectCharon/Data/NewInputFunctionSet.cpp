// Fill out your copyright notice in the Description page of Project Settings.


#include "NewInputFunctionSet.h"


ANewInputFunctionSet::ANewInputFunctionSet()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ANewInputFunctionSet::AddEvent(FGameplayTag Tag, FInputActionDelegate Delegate, ETriggerEvent TriggerEvent)
{
	InputFunctionDelegateMap.Add(Tag, {Delegate, TriggerEvent});
}

