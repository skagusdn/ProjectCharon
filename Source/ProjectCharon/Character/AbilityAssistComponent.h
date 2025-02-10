// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/Attributes/RunAttributeSet.h"
#include "Data/CharacterAbilityConfig.h"
#include "AbilityAssistComponent.generated.h"



class UCharonAbilitySystemComponent;
class UAbilityAssistComponent;

//Attribute값 변경시 호출되는 델리게이트
DECLARE_MULTICAST_DELEGATE_FourParams(FCharonAttributeChanged, UAbilityAssistComponent*, float, float, AActor*);

// DECLARE_MULTICAST_DELEGATE_FourParams(FCharonAttributeChanged, UAbilityAssistComponent*,
// 											  AbilityAssistComp, float, OldValue, float, NewValue, AActor*,
// 											  Instigator);



//Attribute값 변경시 호출되는 델리게이트에 바인드할 이벤트.
DECLARE_DYNAMIC_DELEGATE_FourParams(FCharonSingleAttributeChanged, UAbilityAssistComponent*,
											  AbilityAssistComp, float, OldValue, float, NewValue, AActor*,
											  Instigator);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCHARON_API UAbilityAssistComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAbilityAssistComponent();

	// //deprecated 콜백 함수 바인딩할.  
	// UPROPERTY(BlueprintAssignable)
	// FCharonAttributeChanged OnRunSpeedChanged;
	//
	// UPROPERTY(BlueprintAssignable)
	// FCharonAttributeChanged OnStaminaChanged;
	// //
	
	//각종 초기화.
	void InitAbilityAssist(UCharonAbilitySystemComponent* InAsc, AActor* InOwnerActor, const TObjectPtr<UCharacterAbilityConfig>& InAbilityConfig);
	void InitializeAbilitySystem(UCharonAbilitySystemComponent* InAsc, AActor* InOwnerActor);
	void UninitializeAbilitySystem();
	void InitializeAttributes();
	void UninitializeAttributes();
	
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category="Charon|Ability")
	UCharonAbilitySystemComponent* GetCharonAbilitySystemComponent() const { return Cast<UCharonAbilitySystemComponent>(AbilitySystemComponent);};
	
	UFUNCTION(BlueprintPure, Category= "Charon|Character")
	static UAbilityAssistComponent* FindAbilityAssistComponent(const AActor* Actor){return Actor? Actor->FindComponentByClass<UAbilityAssistComponent>() : nullptr;};

	//Attribute Changed 델리게이트에 바인드 하기.
	UFUNCTION(BlueprintCallable)
	bool BindEventOnAttributeChanged(FGameplayAttribute InAttribute, FCharonSingleAttributeChanged Event);
	
	UFUNCTION(BlueprintCallable)
	void UnbindEventOnAttributeChanged(const FCharonSingleAttributeChanged Event);
	
	//Ability Set을 등록
	void SwitchAbilitySet(TSet<UCharonAbilitySet*> Abilities);
	// Ability 디폴트 값으로 초기화. 
	void ResetToDefaultAbilitySet();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// AbilitySet 해제.
	void ClearAbilitySet();

	//Attribute값 변화시 호출되는 함수. 
	UFUNCTION(Server, Reliable)
	void Server_HandleAttributeChange(UAbilityAssistComponent* AbilityAssistComp, FGameplayAttribute Attribute, float OldValue, float NewValue, AActor* Instigator);
	
	UFUNCTION(Client, Reliable)
	void Client_HandleAttributeChange(UAbilityAssistComponent* AbilityAssistComp, FGameplayAttribute Attribute, float OldValue, float NewValue, AActor* Instigator);

	void HandleAttributeChange(UAbilityAssistComponent* AbilityAssistComp, FGameplayAttribute Attribute, float OldValue, float NewValue, AActor* Instigator);

protected:
	
	UPROPERTY(Replicated)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated)
	TObjectPtr<UCharacterAbilityConfig> DefaultAbilityConfig;

	FCharonAbilitySet_GrantedHandles GrantedAbilityHandles;

	//Attribute 값 바뀌면 broadcast될 델리게이트 모음. (replicated)
	TMap<FGameplayAttribute, FCharonAttributeChanged> AttributeChangedDelegates;
	
	// Attribute Changed 델리게이트 바인드 Handles
	TMap<FCharonSingleAttributeChanged, TPair<FGameplayAttribute,FDelegateHandle>> AttributeChangedBindHandles;
	
};
