// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterAbilityConfig.h"
#include "Data/InputFunctionSet.h"
#include "RiderComponent.generated.h"

//class ANewInputFunctionSet;
struct FVehicleInfo;
class AVehicle;

/* 캐릭터에서 Vehicle 탑승 처리하는 컴포넌트.
 *
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCHARON_API URiderComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URiderComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;
	
public:
	UFUNCTION(BlueprintCallable)
	void HandleRide(AVehicle* Vehicle, UCharacterAbilityConfig* AbilityConfig, AInputFunctionSet* InputFunctions);
		
	UFUNCTION(BlueprintCallable)
	void HandleUnride();
	
	UFUNCTION(BlueprintPure)
	static URiderComponent* FindRiderComponent(ACharacter* Character);

protected:
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TObjectPtr<AVehicle> RidingVehicle;
	
	TObjectPtr<ACharacter> OwnerCharacter;
};
