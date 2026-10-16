// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "AbilitySystem/CharonAbilitySystemComponent.h"
#include "AbilitySystem/CharonAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/Attributes/RunAttributeSet.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "Data/CharacterAbilityConfig.h"
#include "AbilityAssistComponent.generated.h"



class UCharonAbilitySystemComponent;
class UAbilityAssistComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimpleDynamicMulticastDelegate);

//Attribute값 변경시 호출되는 델리게이트
DECLARE_MULTICAST_DELEGATE_FourParams(FCharonAttributeChanged, UAbilityAssistComponent*, float, float, AActor*);


//Attribute값 변경시 호출되는 델리게이트에 바인드할 이벤트.
DECLARE_DYNAMIC_DELEGATE_FourParams(FCharonSingleAttributeChanged, UAbilityAssistComponent*,
											  AbilityAssistComp, float, OldValue, float, NewValue, AActor*, Instigator);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCHARON_API UAbilityAssistComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAbilityAssistComponent(const FObjectInitializer& ObjectInitializer);
	
	//각종 초기화.
	void InitAbilityAssist(UCharonAbilitySystemComponent* InAsc, AActor* InOwnerActor, const TObjectPtr<UCharacterAbilityConfig>& InAbilityConfig);
	void InitializeAbilitySystem(UCharonAbilitySystemComponent* InAsc, AActor* InOwnerActor);
	void UninitializeAbilitySystem();
	
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category="Charon|Ability")
	UCharonAbilitySystemComponent* GetCharonAbilitySystemComponent() const { return Cast<UCharonAbilitySystemComponent>(AbilitySystemComponent);};
	
	UFUNCTION(BlueprintPure, Category= "Charon|Character")
	static UAbilityAssistComponent* FindAbilityAssistComponent(const AActor* Actor){return Actor? Actor->FindComponentByClass<UAbilityAssistComponent>() : nullptr;};
	
	//Ability Set을 등록
	void SwitchAbilitySet(TSet<UCharonAbilitySet*> Abilities);
	// Ability 디폴트 값으로 초기화. 
	void ResetToDefaultAbilitySet();

	UPROPERTY(BlueprintAssignable)
	FAbilityCommitDelegate OnAbilityCommitted;
	
	/** The name of this component-implemented feature */
	static const FName NAME_ActorFeatureName;
	
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface
	
	/** Register with the OnAbilitySystemInitialized delegate and broadcast if our pawn has been registered with the ability system component */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	/** Register with the OnAbilitySystemUninitialized delegate fired when our pawn is removed as the ability system's avatar actor */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);
	
	UPROPERTY(BlueprintAssignable, DisplayName="OnAbilitySystemInitialized")
	FSimpleDynamicMulticastDelegate K2_OnAbilitySystemInitialized;
	
	UPROPERTY(BlueprintAssignable, DisplayName="OnAbilitySystemUninitialized")
	FSimpleDynamicMulticastDelegate K2_OnAbilitySystemUninitialized;
	
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// AbilitySet 해제. (교체용 세트만)
	void ClearAbilitySet();

	// DefaultAbilityConfig의 PersistentAbilities 부여
	void GivePersistentAbilitySets();

	void Server_HandleAbilityCommitted(UGameplayAbility* Ability);
	void HandleAbilityCommitted(FAbilityCommitInfo AbilityCommitInfo);
	UFUNCTION(Client, Reliable)
	void Client_HandleAbilityCommitted(FAbilityCommitInfo AbilityCommitInfo);

	// // TagRelationShip 관련
	// void InitTagRelationship();
	// void OnRelatedTagAddedOrRemoved(FGameplayTag Tag);
	
	
	UPROPERTY(Replicated)
	TObjectPtr<UCharonAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated)
	TObjectPtr<UCharacterAbilityConfig> DefaultAbilityConfig;

	// 교체용 세트 핸들. SwitchAbilitySet / ClearAbilitySet이 다룬다.
	FCharonAbilitySet_GrantedHandles GrantedAbilityHandles;

	// 영구 세트 핸들. 초기화 때 주고 UninitializeAbilitySystem에서만 회수한다.
	FCharonAbilitySet_GrantedHandles PersistentAbilityHandles;

	// UninitializeAbilitySystem 재진입 방지. (어빌리티 취소/회수 중 죽음 종료 콜백 등으로 다시 호출될 수 있음)
	bool bUninitializingAbilitySystem = false;

	// DEPRECATED?
	// //Attribute 값 바뀌면 broadcast될 델리게이트 모음. (replicated)
	// TMap<FGameplayAttribute, FCharonAttributeChanged> AttributeChangedDelegates;

	/** Delegate fired when our pawn becomes the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** Delegate fired when our pawn is removed as the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;
	
	/* DEPRECATED
	// // Attribute Changed 델리게이트 바인드 Handles
	// TMap<FCharonSingleAttributeChanged, TPair<FGameplayAttribute,FDelegateHandle>> AttributeChangedBindHandles;
	*/
};
