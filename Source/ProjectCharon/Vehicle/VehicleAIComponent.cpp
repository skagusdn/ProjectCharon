// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleAIComponent.h"

#include "Vehicle.h"
#include "AI/CharonAIController.h"
#include "AI/CharonAIManager.h"
#include "GameFramework/Character.h"
#include "Interaction/Ability/CharonAbility_Interaction.h"


UVehicleAIComponent::UVehicleAIComponent()
{

}


void UVehicleAIComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnAIRiders();
	for (ACharacter* AIRider : AIRiders)
	{
		ForceAIRide(AIRider);
	}
}

void UVehicleAIComponent::OnRegister()
{
	Super::OnRegister();
	
	if (GetOwner())
	{
		OwnerVehicle = Cast<AVehicle>(GetOwner());
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
		if (UCharonAIManager* AIManager = GetWorld()->GetSubsystem<UCharonAIManager>())
		{
			FVector Location = OwnerVehicle->GetActorLocation();
			Location += {0,0,500};
			FRotator Rotation(0.f, 0.f, 0.f);
			FVector Scale(1.f);

			FTransform Transform(Rotation, Location, Scale);
			
			if (AICharacterClass && AIControllerClass)
			{
				if (ACharacter* SpawnedAICharacter = AIManager->TrySpawnAICharacter(AICharacterClass, Transform, AIControllerClass))
				{
					AIRiders.AddUnique(SpawnedAICharacter);
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

