// Fill out your copyright notice in the Description page of Project Settings.


#include "InputFunctionSet.h"


AInputFunctionSet::AInputFunctionSet()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AInputFunctionSet::AddEvent(FGameplayTag Tag, FInputActionDelegate Delegate, TArray<ETriggerEvent> TriggerEvents, const bool bNeedServerRPC)
{
	InputFunctionDelegateMap.Add(Tag, {Delegate, TriggerEvents, bNeedServerRPC});
}

void AInputFunctionSet::ExecuteInputFunctionByTag(const FInputActionValue& Value, FGameplayTag Tag, ACharacter* Executor)
{
	InputFunctionDelegateMap.Find(Tag)->Delegate.Execute(Value, Executor);
}

