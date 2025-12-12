// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterAbilityConfig.h"
#include "Data/InputFunctionSet.h"
#include "VehicleRiderComponent.generated.h"

class UAttributeBoundWidget;
struct FVehicleUISet;
//class ANewInputFunctionSet;
struct FVehicleInfo;
class AVehicle;

/* 캐릭터에서 Vehicle 탑승 처리하는 컴포넌트.
 *
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCHARON_API UVehicleRiderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVehicleRiderComponent();

protected:
	//virtual void BeginPlay() override;
	virtual void OnRegister() override;
	
public:
	
	// UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	// void ServerHandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig, AInputFunctionSet* InputFunctions, const FVehicleUISet& VehicleUISet);
	//
	// UFUNCTION(Client, Reliable, BlueprintCallable)
	// void ClientHandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig, AInputFunctionSet* InputFunctions, const FVehicleUISet& VehicleUISet);
	//
	// UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	// void ServerHandleUnride();
	//
	// UFUNCTION(Client, Reliable, BlueprintCallable)
	// void ClientHandleUnride();
	//
	UFUNCTION(BlueprintPure)
	static UVehicleRiderComponent* FindRiderComponent(ACharacter* Character);

	UFUNCTION(BlueprintCallable)
	AVehicle* GetRidingVehicle() const {return RidingVehicle;};

	UFUNCTION(BlueprintCallable)
	bool GetIsRidingVehicle() const{ return bIsRidingVehicle; };
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetRidingVehicle(AVehicle* VehicleToRide);
protected:

	
	// //virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// UFUNCTION(BlueprintCallable)
	// void HandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig, AInputFunctionSet* InputFunctions, const FVehicleUISet& VehicleUISet);
	//
	// UFUNCTION(BlueprintCallable)
	// void HandleUnride();
	UFUNCTION()
	void OnRep_RidingVehicle(AVehicle* OldVehicle);
	
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_RidingVehicle, meta=(AllowPrivateAccess=true))
	TObjectPtr<AVehicle> RidingVehicle;
	
	TObjectPtr<ACharacter> OwnerCharacter;

	UPROPERTY(BlueprintReadOnly)
	bool bIsRidingVehicle;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UAnimInstance> OriginalRiderAnimClass;
	// TArray<UAttributeBoundWidget*> VehicleWidgets;
};
