// Fill out your copyright notice in the Description page of Project Settings.


#include "BotCommandAbility.h"

#include "Logging.h"
#include "ShallowWaterSubsystem.h"
#include "AbilitySystem/CharonAbilityUtilities.h"
#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/VehicleAIComponent.h"
#include "Vehicle/VehicleRiderComponent.h"

void UBotCommandAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	InjectAbilityInput();
}

void UBotCommandAbility::InjectAbilityInput()
{
	if (!FindTargetVehicle() || TargetRiderIndex < 0)
	{
		UE_LOG(LogCharon, Warning, TEXT("UBotCommandAbility::InjectAbilityInput, Vehicle or RiderIndex Invalid."));
		return;
	}
	
	check(GetAvatarActorFromActorInfo());
	ACharacter* Rider = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	
	// 목표 Bot 찾고 -> 타겟 어빌리티의 InputTag 찾고 -> InputTag로 입력 주입. 

	const TArray<ACharacter*>& Riders = FindTargetVehicle()->GetRiders();
	if (Riders.Num() <= TargetRiderIndex)
	{
		UE_LOG(LogCharon, Warning, TEXT("UBotCommandAbility::InjectAbilityInput, RiderIndex Invalid."));
		return;
	}

	if (ACharonCharacter* Bot = Cast<ACharonCharacter>(Riders[TargetRiderIndex]))
	{
		if (UAbilitySystemComponent* ASC = Bot->GetAbilitySystemComponent())
		{
			if (FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromClass(TargetRiderAbility))
			{
				// // 발동 전 쿨다운 체크.
				// FAbilityCooldownInfo CooldownInfo = UCharonAbilityUtilities::CheckAbilityStatusAndCooldown(ASC, TargetRiderAbility);
				//
				// ///////
				// UE_LOG(LogTemp, Warning, TEXT("이상해씨 bCanActivate : %s"), CooldownInfo.bCanActivate ? TEXT("true") : TEXT("false"));
				//
				// if (CooldownInfo.bCanActivate)
				// {
				// 	// Input Tag는 Dynamic 태그에 보관중임
				// 	if (!AbilitySpec->GetDynamicSpecSourceTags().IsEmpty())
				// 	{
				// 		FGameplayTag InputTag = AbilitySpec->GetDynamicSpecSourceTags().First();
				//
				// 		UVehicleAIComponent::SendInputCommand(Rider, TargetRiderIndex, FInputActionValue(true), InputTag,
				// 											  true, true);
				// 	
				// 		UE_LOG(LogTemp, Warning, TEXT("UBotCommandAbility::InjectAbilityInput, InputTag : %s"), *InputTag.ToString());
				// 		return;
				// 	}
				// }
				//
				// //발동 신호 후 쿨다운 체크. 
				// CooldownInfo = UCharonAbilityUtilities::CheckAbilityStatusAndCooldown(ASC, TargetRiderAbility);
				// if (CooldownInfo.bIsOnCooldown)
				// {
				// 	UE_LOG(LogTemp, Warning, TEXT("Left Cooldown : %f"), CooldownInfo.TimeRemaining);
				// }
				
				if (UGameplayAbility* Ability = AbilitySpec->Ability)
				{
					float CooldownTimeRemaining, CooldownTimeDuration;
					FGameplayAbilityActorInfo AbilityActorInfo = Ability->GetActorInfo();
					Ability->GetCooldownTimeRemainingAndDuration(AbilitySpec->Handle, &AbilityActorInfo, CooldownTimeRemaining, CooldownTimeDuration);
					
					if (CooldownTimeRemaining == 0.f)
					{
						//Input Tag는 Dynamic 태그에 보관중임
						if (!AbilitySpec->GetDynamicSpecSourceTags().IsEmpty())
						{
							FGameplayTag InputTag = AbilitySpec->GetDynamicSpecSourceTags().First();

							UVehicleAIComponent::SendInputCommand(Rider, TargetRiderIndex, FInputActionValue(true),
							                                      InputTag,
							                                      true, true);
							//
							// UE_LOG(LogTemp, Warning, TEXT("UBotCommandAbility::InjectAbilityInput, InputTag : %s"),
							//        *InputTag.ToString());
							return;
						}
					}
				}
				
				
				
			}
		}
	}
	
	UE_LOG(LogCharon, Warning, TEXT("UBotCommandAbility::InjectAbilityInput Failed."));
	

	
}
