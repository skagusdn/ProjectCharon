// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonCharacter.h"

#include "Data/NewInputFunctionSet.h"
#include "Player/CharonPlayerState.h"


// Sets default values
ACharonCharacter::ACharonCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	AbilityAssistComponent = CreateDefaultSubobject<UAbilityAssistComponent>(TEXT("AbilityAssist"));
	InputAssistComponent = CreateDefaultSubobject<UInputAssistComponent>(TEXT("InputAssist"));
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
	
	//DefaultInputFunctions = CreateDefaultSubobject<ANewInputFunctionSet>(TEXT("DefaultInputFunctions"));
	DefaultInputFunctions = nullptr;
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


UAbilitySystemComponent* ACharonCharacter::GetAbilitySystemComponent() const
{
	if (!AbilityAssistComponent)
	{
		return nullptr;
	}

	return AbilityAssistComponent->GetAbilitySystemComponent();
}

void ACharonCharacter::SwitchAbilityConfig(const UCharacterAbilityConfig* AbilityConfig,
	ANewInputFunctionSet* InputFunctions)
{
	check(AbilityConfig);
	
	if(AbilityAssistComponent)
	{
		if(AbilityConfig->Abilities.Num() > 0)
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
	AbilityAssistComponent->ResetToDefaultAbilitySet();
	InputAssistComponent->ResetToDefaultInputConfig();
}

bool ACharonCharacter::TestIsEqual(UObject* Object1, UObject* Object2)
{
	return Object1 == Object2;
}

void ACharonCharacter::InitCharonCharacter()
{
	// 주의 : 초기화는 다양한 타이밍에 진행되니 여러 번 호출되어도 이상이 없게 디자인하기. ex) 초기화 안하고 능력 부여 여러번되서 이상해졌음. 
	if(ACharonPlayerState* PS = Cast<ACharonPlayerState>(GetPlayerState()))
	{
		AbilityAssistComponent->InitAbilityAssist(PS->GetCharonAbilitySystemComponent(), PS, DefaultAbilityConfig);

		if(InputComponent)
		{
			InputAssistComponent->InitInputAssist(DefaultAbilityConfig->InputConfig, DefaultInputFunctions);
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
	DefaultInputFunctions = GetWorld()->SpawnActor<ANewInputFunctionSet>(DefaultInputFunctionClass);	
}

// Called every frame
void ACharonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

