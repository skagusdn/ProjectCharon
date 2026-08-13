// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleAIComponent.h"

#include "CharonGameplayTags.h"
#include "Vehicle.h"
#include "VehicleRiderComponent.h"
#include "AI/CharonAIController.h"
#include "Character/InputAssistComponent.h"
#include "Character/PawnInitStateComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Framework/CharonBotComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Interaction/Ability/CharonAbility_Interaction.h"
#include "Math/UnitConversion.h"


UVehicleAIComponent::UVehicleAIComponent()
{

}


void UVehicleAIComponent::BeginPlay()
{
	Super::BeginPlay();

	
	SpawnAIRiders();
	
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UGameFrameworkComponentManager* ComponentManager = GameInstance->GetSubsystem<UGameFrameworkComponentManager>();
	if (ComponentManager)
	{
		for (ACharacter* AIRider : AIRiders)
		{
			ComponentManager->RegisterAndCallForActorInitState(AIRider, UPawnInitStateComponent::NAME_ActorFeatureName ,CharonGameplayTags::InitState_GameplayReady, FActorInitStateChangedDelegate::CreateUObject(this, &UVehicleAIComponent::OnBotReady));
			ForceAIRide(AIRider);
		}
		
		
	}
	
	
	
	
	// if(GetWorld())
	// {
	// 	FTimerHandle TimerHandle;
	// 	FTimerDelegate TimerDelegate;
	// 	TimerDelegate.BindLambda([This = this]()->void
	// 	{
	// 		This->SpawnAIRiders();
	// 		
	// 		for (ACharacter* AIRider : This->AIRiders)
	// 		{
	// 			This->ForceAIRide(AIRider);
	// 		}
	// 		
	// 	});
	// 	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 3.f, false);
	// }
	
	
	
}

void UVehicleAIComponent::PushAIInput(ACharacter* Rider, int32 DestinationSeat,
	const FInputActionValue& InputActionValue, FGameplayTag InputTag, bool bIsAbilityAction, bool bIsButtonPressed)
{
	if (!OwnerVehicle ||  OwnerVehicle->FindRiderIdx(Rider) < 0)
	{
		return;
	}
	
	for (ACharacter* AIRider : AIRiders)
	{
		if (OwnerVehicle->FindRiderIdx(AIRider) == DestinationSeat)
		{
			Server_PushAIInput(Rider, DestinationSeat, InputActionValue, InputTag, bIsAbilityAction, bIsButtonPressed);
			
			return;
		}
	}
	
}

UVehicleAIComponent* UVehicleAIComponent::FindVehicleAIComponent(const ACharacter* Rider)
{
	if (UVehicleRiderComponent* RiderComponent =  Rider->FindComponentByClass<UVehicleRiderComponent>())
	{
		if (const AVehicle* Vehicle = RiderComponent->GetRidingVehicle())
		{
			return Vehicle->FindComponentByClass<UVehicleAIComponent>();
		}
	}
	
	return nullptr;
}

void UVehicleAIComponent::OnRegister()
{
	Super::OnRegister();
	
	InputOrderDelegates.Empty();
	if (GetOwner())
	{
		OwnerVehicle = Cast<AVehicle>(GetOwner());
		InputOrderDelegates.AddDefaulted(OwnerVehicle->GetMaxRiderNum());
	}
	else
	{
		OwnerVehicle = nullptr;
	}
	
	if (!OwnerVehicle)
	{
		FText DialogText = FText::FromString(TEXT("컴포넌트가 Vehicle이 아닌 액터에 등록되었습니다."));
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);	
	}
	
}

void UVehicleAIComponent::SpawnAIRiders()
{
	// 일단 임시 테스트 로직
	if (!OwnerVehicle)
	{
		return;
	}
	
	if (!OwnerVehicle->HasAuthority())
	{
		return;
	}
	
	if (GetWorld())
	{
		// if (UCharonAIManager* AIManager = GetWorld()->GetSubsystem<UCharonAIManager>())
		// {
		// 	FVector Location = OwnerVehicle->GetActorLocation();
		// 	Location += {0,0,500};
		// 	FRotator Rotation(0.f, 0.f, 0.f);
		// 	FVector Scale(1.f);
		//
		// 	FTransform Transform(Rotation, Location, Scale);
		// 	
		// 	if (AICharacterClass && AIControllerClass)
		// 	{
		// 		if (ACharacter* SpawnedAICharacter = AIManager->TrySpawnAICharacter(AICharacterClass, Transform, AIControllerClass))
		// 		{
		// 			AIRiders.AddUnique(SpawnedAICharacter);
		// 		}
		// 		
		// 	}
		// 	
		// }
		
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			AGameStateBase* GameState = GameMode->GameState;
			check(GameState);
			
			if (UCharonBotComponent* BotComp = GameState->GetComponentByClass<UCharonBotComponent>())
			{
				FVector Location = OwnerVehicle->GetActorLocation();
				Location += {0,0,500};
				FRotator Rotation(0.f, 0.f, 0.f);
				FVector Scale(1.f);

				FTransform Transform(Rotation, Location, Scale);
			
				if (AICharacterClass && AIControllerClass)
				{
					if (AAIController* SpawnedBot = BotComp->TrySpawnBot(AICharacterClass, Transform, AIControllerClass))
					{
						if (APawn* BotPawn = SpawnedBot->GetPawn())
						{
							if (ACharacter* BotCharacter = Cast<ACharacter>(BotPawn))
							{
								AIRiders.AddUnique(BotCharacter);
							}
						}
					}
				}
			}
			
			
			
		}
		
	}
	
}

void UVehicleAIComponent::ForceAIRide(ACharacter* Rider)
{
	if (!OwnerVehicle || !OwnerVehicle->HasAuthority())
	{
		return;
	}
	
	if (OwnerVehicle->Implements<UInteractiveInterface>())
	{
		if (IAbilitySystemInterface* ASCRider = Cast<IAbilitySystemInterface>(Rider))
		{
			if (UAbilitySystemComponent* ASC = ASCRider->GetAbilitySystemComponent())
			{
				// 상호작용 어빌리티 찾고, 베히클과 강제 상호작용 시키기. 
				for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
				{
					if (Spec.Ability && Spec.Ability->IsA<UCharonAbility_Interaction>())
					{
						if (UCharonAbility_Interaction* InteractionAbility = Cast<UCharonAbility_Interaction>(Spec.GetPrimaryInstance()))
						{
							InteractionAbility->ForceInteraction(OwnerVehicle);		
						}
						
						return;
					}
				}
			}
		}
	}
}

void UVehicleAIComponent::OnBotReady(const FActorInitStateChangedParams& Params)
{
	
	if (Params.FeatureState == CharonGameplayTags::InitState_GameplayReady)
	{
		if (AIRiders.Contains(Params.OwningActor))
		{
			if (ACharacter* AIRider = Cast<ACharacter>(Params.OwningActor))
			{
				ForceAIRide(AIRider);	
			}
					
		}

		// 4. 정리(Cleanup):
		//    더 이상 알림을 받을 필요가 없으므로, 등록을 해제하여 메모리 누수를 방지합니다.
		// ... UnregisterObserver 호출 ...
	}
}

int32 UVehicleAIComponent::FindRiderIdx(AActor* Rider) const
{
	if (ACharacter* RiderCharacter = Cast<ACharacter>(Rider))
	{
		return OwnerVehicle->FindRiderIdx(RiderCharacter);
	}
	
	return -1;
}

void UVehicleAIComponent::Server_PushAIInput_Implementation(ACharacter* Rider, int32 DestinationSeat,
	const FInputActionValue& InputActionValue, FGameplayTag InputTag, bool bIsAbilityAction, bool bIsButtonPressed)
{
	if (!OwnerVehicle ||  OwnerVehicle->FindRiderIdx(Rider) < 0)
	{
		return;
	}
	
	for (ACharacter* AIRider : AIRiders)
	{
		if (OwnerVehicle->FindRiderIdx(AIRider) == DestinationSeat)
		{
			if (InputOrderDelegates.Num() > DestinationSeat)
			{
				// 어차피 이 함수는 서버에서 호출되니 Server RPC 여부는.. 아마 필요 없을듯?
				InputOrderDelegates[DestinationSeat].Broadcast(InputActionValue, InputTag, bIsAbilityAction, bIsButtonPressed, false);
				return;
			}
		}
	}
	
}


