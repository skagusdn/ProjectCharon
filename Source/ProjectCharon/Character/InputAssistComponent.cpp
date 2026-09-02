// Fill out your copyright notice in the Description page of Project Settings.


#include "InputAssistComponent.h"

#include "AbilityAssistComponent.h"
#include "AIController.h"
#include "CharonCharacter.h"
#include "CharonGameplayTags.h"
#include "PawnInitStateComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Data/InputFunctionSet.h"
#include "GameFramework/Character.h"//
#include "Input/CharonInputComponent.h"
#include "Player/CharonController.h"
#include "Player/CharonPlayerState.h"
#include "Vehicle/VehicleAIComponent.h"

const FName UInputAssistComponent::NAME_ActorFeatureName("InputAssist");

// Sets default values for this component's properties
UInputAssistComponent::UInputAssistComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

bool UInputAssistComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == CharonGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	else if (CurrentState == CharonGameplayTags::InitState_Spawned && DesiredState == CharonGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<ACharonPlayerState>())
		{
			return false;
		}

		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
				(Controller->PlayerState != nullptr) && \
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			ACharonController* CharonPC = GetController<ACharonController>();
			
			// 입력 컴포넌트가 없는데 AI도 아닌 경우 False
			if (!Pawn->InputComponent || !CharonPC || !CharonPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == CharonGameplayTags::InitState_DataAvailable && DesiredState == CharonGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		ACharonPlayerState* LyraPS = GetPlayerState<ACharonPlayerState>();

		return LyraPS && Manager->HasFeatureReachedInitState(Pawn, UPawnInitStateComponent::NAME_ActorFeatureName, CharonGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == CharonGameplayTags::InitState_DataInitialized && DesiredState == CharonGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UInputAssistComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	
	if (CurrentState == CharonGameplayTags::InitState_DataAvailable && DesiredState == CharonGameplayTags::InitState_DataInitialized)
	{
		APawn* Pawn = GetPawn<APawn>();
		ACharonPlayerState* CharonPS = GetPlayerState<ACharonPlayerState>();
		if (!ensure(Pawn && CharonPS))
		{
			return;
		}
		
		if (GetController<ACharonController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				// 임시
				if (ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(Pawn))
				{
					InitInputAssist(CharonCharacter->DefaultAbilityConfig->InputConfig, CharonCharacter->DefaultInputFunctions);	
				}
			}
		}
		
	}
	
}

void UInputAssistComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{

	if (Params.FeatureName == UPawnInitStateComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == CharonGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
	
}

void UInputAssistComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { CharonGameplayTags::InitState_Spawned, CharonGameplayTags::InitState_DataAvailable, CharonGameplayTags::InitState_DataInitialized, CharonGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UInputAssistComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UPawnInitStateComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(CharonGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UInputAssistComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

void UInputAssistComponent::HandleInputActionTriggered(const FInputActionValue& ActionValue, FGameplayTag InputTag,
                                                       bool bIsAbilityAction, bool bIsButtonPressed, bool bNeedServerRPC)
{
	if (bIsAbilityAction)
	{
		if (bIsButtonPressed)
		{
			Input_AbilityInputTagPressed(InputTag);
		}
		else
		{
			Input_AbilityInputTagReleased(InputTag);
		}
	}
	else
	{
		RequestExecuteInputFunction(ActionValue, InputTag, bNeedServerRPC);
	}
}

void UInputAssistComponent::OnUnregister()
{
	UnregisterInputConfig(false);
	Super::OnUnregister();
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

void UInputAssistComponent::SwitchInputConfig_Implementation(const UCharonInputConfig* InputConfig, AInputFunctionSet* InInputFunctions, bool bIsTemporary)
{
	const AActor* OwnerActor = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);

	check(OwnerPawn);
	
	if(!OwnerPawn->IsLocallyControlled() && !OwnerPawn->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("call RegisterInputConfig only in local! ^___^"));
		return;
	}
	
	if(!InputConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("InputConfig is Not Valid"));
		return;
	}

	if(!bIsTemporary && TempActiveInputConfig == nullptr && ActiveInputConfig == InputConfig)
	{
		return;
	}
		
	if(ActiveInputConfig)
	{
		UnregisterInputConfig(bIsTemporary);
	}

	if(!bIsTemporary)
	{
		ActiveInputConfig = InputConfig;
		ActiveInputFunctions = InInputFunctions;

		TempActiveInputConfig = nullptr;
		TempActiveInputFunctions = nullptr;
	}
	else
	{
		TempActiveInputConfig = InputConfig;
		TempActiveInputFunctions = InInputFunctions;
	}

	// 로컬 플레이어가 아닌 서버는 밑에 바인딩 작업은 할필요 x
	if (!OwnerPawn->IsLocallyControlled())
	{
		return;
	}
	
	// ai인 경우 입력 컴포넌트가 아닌 AI 컴포넌트에 바인드
	// TODO : 아직 AI 캐릭터가 베히클에서 내린 경우는 계획하지 않음.  
	if (OwnerPawn->GetController()->IsA<AAIController>())
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			if (UVehicleAIComponent* VehicleAIComp = UVehicleAIComponent::FindVehicleAIComponentFromRider(OwnerCharacter))
			{
				VehicleAIComp->RegisterAIInputReceiver(this, &UInputAssistComponent::HandleInputActionTriggered);
			}	
		}
		
		return;
	}
	
	
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
			
		// CharonIC->BindAbilityActions(InputConfig, this, &UInputAssistComponent::Input_AbilityInputTagPressed,
		// 	&UInputAssistComponent::Input_AbilityInputTagReleased, AbilityBindHandles);
		CharonIC->BindAbilityActions(InputConfig, AbilityBindHandles);

		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		
		if(InInputFunctions && OwnerCharacter)
		{
			//CharonIC->BindNativeFunctions(InputConfig, OwnerCharacter, InInputFunctions,  NativeBindHandles );
			CharonIC->BindNativeFunctions(InputConfig, this, InInputFunctions, NativeBindHandles );
		}
		
		//
		//CharonIC->OnInputActionTriggered.AddDynamic(this, &UInputAssistComponent::HandleInputActionTriggered);
		CharonIC->RegisterInputReceiver(this, &UInputAssistComponent::HandleInputActionTriggered);
		 
	}
}

void UInputAssistComponent::UnregisterInputConfig(bool bIsTemporary)
{
	const AActor* OwnerActor = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);

	check(OwnerPawn);

	const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if(!PC)
	{
		return;
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer(); 
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem);

	UCharonInputComponent* CharonIC =  Cast<UCharonInputComponent>(OwnerPawn->InputComponent);

	if(ensureMsgf(CharonIC, TEXT("This Input Component Is Not SubClass of CharonInputComponent!")))
	{
		// TODO : 이거 이렇게 하면 여러 InputConfig에서 겹치는 InputMappingContext가 있으면 문제가 되겠는데???
		CharonIC->RemoveInputMappings( ActiveInputConfig, InputSubsystem);
			
		CharonIC->RemoveBinds(AbilityBindHandles);
		CharonIC->RemoveBinds(NativeBindHandles);
	}

	//임시 교체일 경우 PresentInputConfig는 유지. 
	if(!bIsTemporary)
	{
		ActiveInputConfig = nullptr;	
	}
	
	
}

void UInputAssistComponent::K2_SwitchTemporaryInputConfig(const UCharonInputConfig* InputConfig,
	AInputFunctionSet* InputFunctions)
{
	SwitchInputConfig(InputConfig, InputFunctions, true);
}

void UInputAssistComponent::K2_UnregisterTemporaryInputConfig(const UCharonInputConfig* InputConfig, AInputFunctionSet* InputFunctions)
{
	if(APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		if(PawnOwner->IsLocallyControlled())
		{
			
		}
	}
	
	
	SwitchInputConfig(ActiveInputConfig, ActiveInputFunctions, false);
}

void UInputAssistComponent::ResetToDefaultInputConfig_Implementation()
{
	if(ActiveInputConfig)
	{
		if(ActiveInputConfig == DefaultInputConfig)
		{
			return;
		}

		UnregisterInputConfig(false);
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

// void UInputAssistComponent::RequestExecuteInputFunction(FInputActionValue InputActionValue,
// 	AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag, bool IsServerRPC)
// {
// 	// 전달받은 InputFunctionSet이 적절한지 검사
// 	if (TempActiveInputFunctions)
// 	{
// 		if (TempActiveInputFunctions != InputFunctionSet)
// 		{
// 			return;
// 		}
// 	}
// 	else
// 	{
// 		if (!ActiveInputFunctions || ActiveInputFunctions != InputFunctionSet)
// 		{
// 			return;
// 		}
// 	}
// 	
// 	
// 	if(IsServerRPC)
// 	{
// 		Server_RequestExecuteInputFunction(InputActionValue[0], InputActionValue[1], InputActionValue[2],
// 			InputActionValue.GetValueType(), InputFunctionSet, Tag);
// 	}
// 	else
// 	{
// 		// TODO : 굳이 캐릭터로 변환해서 줘야할까?
// 		if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
// 		{
// 			InputFunctionSet->ExecuteInputFunctionByTag(InputActionValue, Tag, OwningCharacter);
// 		}
// 	}
// }

void UInputAssistComponent::RequestExecuteInputFunction(FInputActionValue InputActionValue, const FGameplayTag Tag,
	bool IsServerRPC)
{
	
	AInputFunctionSet* TargetInputFunctionSet = TempActiveInputFunctions != nullptr ? TempActiveInputFunctions : ActiveInputFunctions;
	
	if(IsServerRPC)
	{
		Server_RequestExecuteInputFunction(InputActionValue[0], InputActionValue[1], InputActionValue[2],
			InputActionValue.GetValueType(), TargetInputFunctionSet, Tag);
	}
	else if (TargetInputFunctionSet)
	{
		// TODO : 굳이 캐릭터로 변환해서 줘야할까?
		if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
		{
			TargetInputFunctionSet->ExecuteInputFunctionByTag(InputActionValue, Tag, OwningCharacter);
		}
	}
}

void UInputAssistComponent::Server_RequestExecuteInputFunction_Implementation(float ValueX, float ValueY, float ValueZ,
	EInputActionValueType ValueType, AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag)
{
	FInputActionValue InputActionValue;
	switch (ValueType)
	{
	case EInputActionValueType::Boolean:
		InputActionValue = FInputActionValue(ValueX > 0);
		break;
	case EInputActionValueType::Axis1D:
		InputActionValue = FInputActionValue(ValueX);
		break;
	case EInputActionValueType::Axis2D:
		InputActionValue = FInputActionValue(FVector2D(ValueX, ValueY));
		break;
	case EInputActionValueType::Axis3D:
		InputActionValue = FInputActionValue(FVector(ValueX, ValueY, ValueZ));
		break;
	}
		
	
	//RequestExecuteInputFunction(InputActionValue, InputFunctionSet, Tag, false);
	if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
	{
		InputFunctionSet->ExecuteInputFunctionByTag(InputActionValue, Tag, OwningCharacter);
	}
}






