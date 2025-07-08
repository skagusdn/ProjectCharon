// Fill out your copyright notice in the Description page of Project Settings.


#include "InputAssistComponent.h"

#include "AbilityAssistComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "GameFramework/Character.h"//
#include "Input/CharonInputComponent.h"



// Sets default values for this component's properties
UInputAssistComponent::UInputAssistComponent()
{
	
}


// Called when the game starts
void UInputAssistComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}



void UInputAssistComponent::ClearAbilityConfig()
{
	//
}


void UInputAssistComponent::InitInputAssist(const UCharonInputConfig* InputConfig, AInputFunctionSet* InInputFunctions)
{

	if(InputConfig)
	{
		DefaultInputConfig = InputConfig;
		DefaultInputFunctions = InInputFunctions;
		
		ResetToDefaultInputConfig();
	}
}

void UInputAssistComponent::SwitchInputConfig_Implementation(const UCharonInputConfig* InputConfig, AInputFunctionSet* InInputFunctions)
{
	const AActor* OwnerActor = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);

	check(OwnerPawn);
	
	if(!OwnerPawn->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("call RegisterInputConfig only in local! ^___^"));
		return;
	}
	
	if(!InputConfig || PresentInputConfig == InputConfig)
	{
		return;
	}
		
	if(PresentInputConfig)
	{
		UnregisterInputConfig();
	}

	PresentInputConfig = InputConfig;

	const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer(); // 아 맞다 서버에선 로컬 플레이어를 못 얻는구나;
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem);

	UCharonInputComponent* CharonIC =  Cast<UCharonInputComponent>(OwnerPawn->InputComponent);

	//어빌리티 및 함수 바인드
	if(ensureMsgf(CharonIC, TEXT("Input Component Is Not SubClass of CharonInputComponent!")))
	{
		CharonIC->AddInputMappings(InputConfig, InputSubsystem);
			
		CharonIC->BindAbilityActions(InputConfig, this, &UInputAssistComponent::Input_AbilityInputTagPressed,
			&UInputAssistComponent::Input_AbilityInputTagReleased, AbilityBindHandles);

		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		
		if(InInputFunctions && OwnerCharacter)
		{
			CharonIC->BindNativeFunctions(InputConfig, OwnerCharacter, InInputFunctions,  NativeBindHandles );
		}
	}
	
	
}

void UInputAssistComponent::UnregisterInputConfig()
{
	const AActor* OwnerActor = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);

	check(OwnerPawn);

	const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer(); 
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem);

	UCharonInputComponent* CharonIC =  Cast<UCharonInputComponent>(OwnerPawn->InputComponent);

	if(ensureMsgf(CharonIC, TEXT("Input Component Is Not SubClass of CharonInputComponent!")))
	{
		// TODO : 이거 이렇게 하면 여러 InputConfig에서 겹치는 InputMappingContext가 있으면 문제가 되겠는데???
		CharonIC->RemoveInputMappings( PresentInputConfig, InputSubsystem);
			
		CharonIC->RemoveBinds(AbilityBindHandles);
		CharonIC->RemoveBinds(NativeBindHandles);
	}
	
	PresentInputConfig = nullptr;
	
}

void UInputAssistComponent::ResetToDefaultInputConfig_Implementation()
{
	if(PresentInputConfig)
	{
		if(PresentInputConfig == DefaultInputConfig)
		{
			return;
		}

		UnregisterInputConfig();
	}

	SwitchInputConfig(DefaultInputConfig, DefaultInputFunctions);
}


void UInputAssistComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const AActor* Owner = GetOwner())
	{
		if (const UAbilityAssistComponent* AbilityAssist = UAbilityAssistComponent::FindAbilityAssistComponent(Owner))
		{
			if (UAbilitySystemComponent* ASC = AbilityAssist->GetAbilitySystemComponent())
			{
				// 목표는 : 현재 부여된 어빌리티 중 해당 태그를 가지고 있는 어빌리티 스펙을 찾아서,
				// 해당 어빌리티의 InputPressed를 호출하는거임. 
			
				if(UCharonAbilitySystemComponent* CharonASC = Cast<UCharonAbilitySystemComponent>(ASC))
				{
					CharonASC->AbilityLocalInputTagPressed(InputTag);
				}
			}
		}	
	}
}

void UInputAssistComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (const AActor* Owner = GetOwner())
	{
		if (const UAbilityAssistComponent* AbilityAssist = UAbilityAssistComponent::FindAbilityAssistComponent(Owner))
		{
			if (UAbilitySystemComponent* ASC = AbilityAssist->GetAbilitySystemComponent())
			{
				if(UCharonAbilitySystemComponent* CharonASC = Cast<UCharonAbilitySystemComponent>(ASC))
				{
					CharonASC->AbilityLocalInputTagReleased(InputTag);
				}
								
			}
		}	
	}
}







