// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonDamageExecution.h"

#include "AbilitySystem/Attributes/CombatAttributeSet.h"
#include "AbilitySystem/Attributes/HealthAttributeSet.h"


struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition DamageAmountCapture;
	
	FDamageStatics()
	{
		DamageAmountCapture = FGameplayEffectAttributeCaptureDefinition(UCombatAttributeSet::GetDamageAmountAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

UCharonDamageExecution::UCharonDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().DamageAmountCapture);
}

void UCharonDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

#if WITH_SERVER_CODE

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float DamageAmount = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageAmountCapture, EvaluateParameters, DamageAmount);
	
	if(DamageAmount > 0)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UHealthAttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageAmount));
	}

#endif
}
