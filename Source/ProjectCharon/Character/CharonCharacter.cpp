// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonCharacter.h"

#include "CharonCharacterMovementComponent.h"
#include "LifeStateComponent.h"
#include "Data/InputFunctionSet.h"
#include "Player/CharonPlayerState.h"



// ACharonCharacter::ACharonCharacter()
// {
// 	PrimaryActorTick.bCanEverTick = true;
// 	bReplicates = true;
//
// 	AbilityAssistComponent = CreateDefaultSubobject<UAbilityAssistComponent>(TEXT("AbilityAssist"));
// 	InputAssistComponent = CreateDefaultSubobject<UInputAssistComponent>(TEXT("InputAssist"));
// 	DefaultInputFunctions = nullptr;
//
// 	LifeStateComponent = CreateDefaultSubobject<ULifeStateComponent>(TEXT("LifeStateComponent"));
// 	LifeStateComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
// 	LifeStateComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
// }

ACharonCharacter::ACharonCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCharonCharacterMovementComponent>(ACharacter::CharacterMovementComponentName ))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	AbilityAssistComponent = CreateDefaultSubobject<UAbilityAssistComponent>(TEXT("AbilityAssist"));
	InputAssistComponent = CreateDefaultSubobject<UInputAssistComponent>(TEXT("InputAssist"));
	DefaultInputFunctions = nullptr;
	
	LifeStateComponent = CreateDefaultSubobject<ULifeStateComponent>(TEXT("LifeStateComponent"));
	LifeStateComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	LifeStateComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
}

void ACharonCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	InitCharonCharacter();
}

void ACharonCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitCharonCharacter();
}

void ACharonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InitCharonCharacter();
}

void ACharonCharacter::OnDeathStarted(AActor* OwningActor)
{
	if(OwningActor != this)
	{
		return;
	}

	K2_OnDeathStarted();
}

void ACharonCharacter::OnDeathFinished(AActor* OwningActor)
{
	if(OwningActor != this)
	{
		return;
	}

	K2_OnDeathFinished();
	UninitCharonCharacter();
}

void ACharonCharacter::UninitCharonCharacter()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	InputAssistComponent->UnregisterInputConfig(false);
	LifeStateComponent->UninitializeFromAbilitySystem();
	
	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (UCharonAbilitySystemComponent* CharonASC = Cast<UCharonAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		if (CharonASC->GetAvatarActor() == this)
		{
			AbilityAssistComponent->UninitializeAbilitySystem();
		}
	}

	SetActorHiddenInGame(true);
}


UAbilitySystemComponent* ACharonCharacter::GetAbilitySystemComponent() const
{
	if (!AbilityAssistComponent)
	{
		return nullptr;
	}

	return AbilityAssistComponent->GetAbilitySystemComponent();
}

void ACharonCharacter::SwitchAbilityConfig(const UCharacterAbilityConfig* AbilityConfig,
	AInputFunctionSet* InputFunctions)
{
	check(AbilityConfig);
	
	if(AbilityAssistComponent)
	{
		if(AbilityConfig->Abilities.Num() > 0 && HasAuthority())
		{
			AbilityAssistComponent->SwitchAbilitySet(AbilityConfig->Abilities);
		}

		if(const UCharonInputConfig * InputConfig = AbilityConfig -> InputConfig)
		{
			InputAssistComponent->SwitchInputConfig(InputConfig, InputFunctions);
		}
		
	}
	
}

void ACharonCharacter::ResetAbilityConfig()
{
	if(HasAuthority())
	{
		AbilityAssistComponent->ResetToDefaultAbilitySet();
		InputAssistComponent->ResetToDefaultInputConfig();
	}
	
}

void ACharonCharacter::RequestExecuteInputFunction(FInputActionValue InputActionValue, AInputFunctionSet* InputFunctionSet,
	const FGameplayTag Tag, bool IsServerRPC)
{
	if(IsServerRPC)
	{
		//FVector Value = InputActionValue.Get<FVector>();
		Server_RequestExecuteInputFunction(InputActionValue[0], InputActionValue[1], InputActionValue[2],
			InputActionValue.GetValueType(), InputFunctionSet, Tag, false);
	}

	InputFunctionSet->ExecuteInputFunctionByTag(InputActionValue, Tag, this);
	
}

void ACharonCharacter::Server_RequestExecuteInputFunction_Implementation(float ValueX, float ValueY, float ValueZ,
	EInputActionValueType ValueType, AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag, bool IsServerRPC)
{
	FInputActionValue InputActionValue;
	switch(ValueType)
	{
	case EInputActionValueType::Boolean :
		InputActionValue = FInputActionValue(ValueX > 0);
		break;
	case EInputActionValueType::Axis1D :
		InputActionValue = FInputActionValue(ValueX);
		break;
	case EInputActionValueType::Axis2D :
		InputActionValue = FInputActionValue(FVector2D(ValueX, ValueY));
		break;
	case EInputActionValueType::Axis3D :
		InputActionValue = FInputActionValue(FVector(ValueX, ValueY, ValueZ));
		break;
	}
	
	RequestExecuteInputFunction(InputActionValue, InputFunctionSet, Tag, IsServerRPC);
}

// void ACharonCharacter::Server_RequestExecuteInputFunction_Implementation(FInputActionValue InputActionValue,
//                                                                          AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag, bool IsServerRPC)
// {
// 	RequestExecuteInputFunction(InputActionValue, InputFunctionSet, Tag, IsServerRPC);
// }


void ACharonCharacter::InitCharonCharacter()
{
	// 주의 : 초기화는 다양한 타이밍에 진행되니 여러 번 호출되어도 이상이 없게 디자인하기. ex) 부여한 능력 회수 안하고 능력 부여 여러번 되는 경우. 
	if(ACharonPlayerState* PS = Cast<ACharonPlayerState>(GetPlayerState()))
	{
		AbilityAssistComponent->InitAbilityAssist(PS->GetCharonAbilitySystemComponent(), PS, DefaultAbilityConfig);

		if(InputComponent)
		{
			InputAssistComponent->InitInputAssist(DefaultAbilityConfig->InputConfig, DefaultInputFunctions);
		}

		if(UCharonAbilitySystemComponent* CharonASC = Cast<UCharonAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			LifeStateComponent->InitializeWithAbilitySystem(CharonASC);	
		}
		
	}
	
}

void ACharonCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitCharonCharacter();
}

// void ACharonCharacter::PawnClientRestart()
// {
// 	Super::PawnClientRestart();
//
// 	InitCharonCharacter();
// }




// Called when the game starts or when spawned
void ACharonCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACharonCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// InputFunctionSet에 함수 등록.
	//InitInputFunctions();

	//일단 얘는 리플리케이션 꺼둘거니까 서버랑 클라이언트 각자 생성하는걸로.
	DefaultInputFunctions = GetWorld()->SpawnActor<AInputFunctionSet>(DefaultAbilityConfig->InputFunctionSetClass);	
}

// Called every frame
void ACharonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

