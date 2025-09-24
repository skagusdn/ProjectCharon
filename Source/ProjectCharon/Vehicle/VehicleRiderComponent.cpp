// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicle/VehicleRiderComponent.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Character/CharonCharacter.h"
#include "GameFramework/Character.h"
#include "UI/AttributeBoundWidget.h"
#include "Vehicle/Vehicle.h"

// Sets default values for this component's properties
UVehicleRiderComponent::UVehicleRiderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	RidingVehicle = nullptr;
	OwnerCharacter = nullptr;
}


// void UVehicleRiderComponent::BeginPlay()
// {
// 	Super::BeginPlay();
// 	
//
// }

void UVehicleRiderComponent::OnRegister()
{
	Super::OnRegister();
	AActor* Owner = GetOwner();
	
	OwnerCharacter = Cast<ACharacter>(Owner);
}

// void UVehicleRiderComponent::HandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig,
// 	AInputFunctionSet* InputFunctions, const FVehicleUISet& VehicleUISet)
// {
// 	check(Vehicle);
//
// 	RidingVehicle = Vehicle;
// 	if(ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(OwnerCharacter))
// 	{
// 		// Ability 및 입력 등록
// 		if(AbilityConfig)
// 		{
// 			CharonCharacter->SwitchAbilityConfig(AbilityConfig, InputFunctions);	
// 		}
// 	}
// }
//
// void UVehicleRiderComponent::ServerHandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig,
// 	AInputFunctionSet* InputFunctions, const FVehicleUISet& VehicleUISet)
// {
// 	if(!GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
// 	HandleRide(Vehicle, AbilityConfig, InputFunctions, VehicleUISet);
// 	ClientHandleRide(Vehicle, AbilityConfig, InputFunctions, VehicleUISet);
// }
//
// void UVehicleRiderComponent::ServerHandleUnride()
// {
// 	if(!GetOwner()->HasAuthority())
// 	{
// 		return;
// 	}
// 	HandleUnride();
// 	ClientHandleUnride();
// }
//
//
// void UVehicleRiderComponent::ClientHandleRide_Implementation(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig,
//                                                       AInputFunctionSet* InputFunctions, const FVehicleUISet& VehicleUISet)
// {
// 	HandleRide(Vehicle, AbilityConfig, InputFunctions, VehicleUISet);
//
// 	// Vehicle UI 적용.
// 	if(VehicleUISet.WidgetClassList.Num() > 0)
// 	{
// 		APawn* Owner = Cast<APawn>(GetOwner());
// 		UCharonAbilitySystemComponent* VehicleASC = Cast<UCharonAbilitySystemComponent>(Vehicle->GetAbilitySystemComponent());
// 		for(TSubclassOf<UAttributeBoundWidget> WidgetClass : VehicleUISet.WidgetClassList)
// 		{
// 			UAttributeBoundWidget* Widget = CreateWidget<UAttributeBoundWidget>(Cast<APlayerController>(Owner->Controller), WidgetClass);
// 			if(Widget)
// 			{
// 				Widget->InitAttributeBoundWidget(VehicleASC);
// 				Widget->AddToViewport();
// 				VehicleWidgets.Add(Widget);
// 			}
// 		}
// 	}
// }
//
// void UVehicleRiderComponent::ClientHandleUnride_Implementation()
// {
// 	HandleUnride();
//
// 	// Vehicle UI 해제
// 	for(UAttributeBoundWidget* Widget : VehicleWidgets)
// 	{
// 		Widget->RemoveFromParent();
// 	}
// 	VehicleWidgets.Empty();
// 	
// }
//
// void UVehicleRiderComponent::HandleUnride()
// {
// 	if(ACharonCharacter* CharonCharacter = Cast<ACharonCharacter>(OwnerCharacter))
// 	{
// 		CharonCharacter->ResetAbilityConfig();
// 	}
//
// 	RidingVehicle = nullptr;
// }


UVehicleRiderComponent* UVehicleRiderComponent::FindRiderComponent(ACharacter* Character)
{
	check(Character);
	return Character->FindComponentByClass<UVehicleRiderComponent>();
}





