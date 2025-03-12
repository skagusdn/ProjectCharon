// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//#include "InteractiveInterface.h"
#include "AbilitySystemInterface.h"
#include "Data/InputFunctionSet.h"
#include "Interaction/InteractiveInterface.h"
#include "Vehicle.generated.h"

class UCharacterAbilityConfig;
class UCharonInputConfig;
class UCharonAbilitySet;
class UGameplayAbility;

//////////////
USTRUCT(BlueprintType)
struct FTestStruct
{
	GENERATED_BODY()
public :
	TArray<TObjectPtr<AInputFunctionSet>> TestFunctions;
};
//////////

UCLASS()
class PROJECTCHARON_API AVehicle : public AActor, public IInteractiveInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVehicle();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<ACharacter>> Riders;

	// UFUNCTION(BlueprintCallable)
	// int32 GetCurrentRiderNum();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<TObjectPtr<USceneComponent>> Seats;

	UFUNCTION(BlueprintCallable)
	int32 Ride(ACharacter* Rider);

	UFUNCTION(BlueprintCallable)
	bool UnRide(ACharacter* Rider);

	UFUNCTION(BlueprintCallable)
	int32 FindRiderIdx(ACharacter* Rider);


	//~ IAbilitySystemInterface 시작
	/** 어빌리티 시스템 컴포넌트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝


	
protected:

	// UFUNCTION(BlueprintImplementableEvent)
	// void InitInputFunctions();
	
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//에디터에서 프로퍼티 바꿀때 호출.
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRiderNum = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxRiderNum = 1;
	
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	//얘는 일단 나중에. TEST 중~~~~~~~~~~~~~~~~~~~~~~~~~~
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<UCharacterAbilityConfig*> AbilityConfigsForRiders;

	UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<AInputFunctionSet>> InputFunctionSets;

	UPROPERTY(EditAnywhere, Category = "Charon|Input")
	TArray<TSubclassOf<AInputFunctionSet>> InputFunctionSetClasses;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
};
