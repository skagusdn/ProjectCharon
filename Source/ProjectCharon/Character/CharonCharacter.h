// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Net/UnrealNetwork.h"//
#include "GameFramework/Character.h"
#include "AbilityAssistComponent.h"
#include "InputAssistComponent.h"
#include "Interaction/InteractionComponent.h"


#include "CharonCharacter.generated.h"


class AInputFunctionSet;
class UCharacterAbilityConfig;

UCLASS()
class PROJECTCHARON_API ACharonCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharonCharacter();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	

	// UFUNCTION(BlueprintImplementableEvent)
	// void InitInputFunctions();

	virtual void PossessedBy(AController* NewController) override;
	//virtual void PawnClientRestart() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	//어빌리티 쪽 처리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true", RenameComponentFrom = "ExpComp"))
	TObjectPtr <UAbilityAssistComponent> AbilityAssistComponent;

	//입력 관련 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAssistComponent> InputAssistComponent;

	//상호작용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInteractionComponent> InteractionComponent;
	
	//InputAction과 매핑할 InputFunction
	UPROPERTY(BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AInputFunctionSet> DefaultInputFunctions;

	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCharacterAbilityConfig> DefaultAbilityConfig;

	//각종 컴포넌트, 입력, ASC 등 초기화.
	virtual void InitCharonCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//~ IAbilitySystemInterface 시작
	/** 어빌리티 시스템 컴포넌트를 반환합니다. */
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ IAbilitySystemInterface 끝

	UFUNCTION(BlueprintCallable)
	void SwitchAbilityConfig(const UCharacterAbilityConfig* AbilityConfig, AInputFunctionSet* InputFunctions );
	
	UFUNCTION(BlueprintCallable)
	void ResetAbilityConfig();

	//InputFunctionSet의 함수 델리게이트를 실행 요청.	
	UFUNCTION()
	void RequestExecuteInputFunction(FInputActionValue InputActionValue, AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag, bool IsServerRPC);
	
	// RPC에서 FInputActionValue이 제대로 전달되지 않으므로 값을 분리해서 전달. 
	UFUNCTION(Server, Reliable)
	void Server_RequestExecuteInputFunction(float ValueX, float ValueY, float ValueZ, EInputActionValueType ValueType, AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag, bool IsServerRPC);
	//void Server_RequestExecuteInputFunction(FInputActionValue InputActionValue, AInputFunctionSet* InputFunctionSet, const FGameplayTag Tag, bool IsServerRPC);
	
		
};
