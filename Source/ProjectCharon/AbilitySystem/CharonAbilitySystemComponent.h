// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/GarbageCollectionSchema.h"
#include "CharonAbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCharonAttributeChanged, float, float);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCharonAttributeChanged_Dynamic, float, OldValue, float, NewValue);

USTRUCT(BlueprintType)
struct FDelegateHandleWrapper
{
	GENERATED_BODY()

	FDelegateHandleWrapper(){};
	FDelegateHandleWrapper(FDelegateHandle InHandle, TObjectPtr<UObject> InEventSourcePtr)
	: Handle(InHandle), EventSourcePtr(InEventSourcePtr) {};

	bool IsValid() const { return Handle.IsValid(); }

	FDelegateHandle Handle;
	TObjectPtr<UObject> EventSourcePtr;
	
	const bool operator==(const FDelegateHandleWrapper& Other)
	{
		return Handle == Other.Handle;	
	}	
};

/**
 * 커스텀 Ability System Component
 */
UCLASS()
class PROJECTCHARON_API UCharonAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public :
	UCharonAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	// 어트리뷰트 값 변화 델리게이트에 바인딩/언바인딩 하기. 
	// TODO : 나중에 Bind/UnBind는 BlueprintCallable을 없애고 다른 c++클래스 경유해서 하게 하는게 나을듯?  
	UFUNCTION(BlueprintCallable, Category = "Charon | Ability", DisplayName="BindEventOnAttributeChange")
	FDelegateHandleWrapper K2_BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged_Dynamic Event);
	FDelegateHandleWrapper BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged Event);
	
	UFUNCTION(BlueprintCallable, Category = "Charon | Ability", DisplayName="UnBindEventOnAttributeChange")
	void K2_UnBindEventOnAttributeChange(const FDelegateHandleWrapper EventHandle);
	void UnBindEventOnAttributeChange(const FDelegateHandleWrapper EventHandle);
	

	//void UnBindEventOnAttributeChange(FDelegateHandle EventHandle);
	

	//
	UFUNCTION(BlueprintCallable, meta=(DevelopmentOnly))
	void CheckAttributeBinds();
	//
	
	// Input
	void AbilityLocalInputTagPressed(FGameplayTag InputTag);
	void AbilityLocalInputTagReleased(FGameplayTag InputTag);

protected:

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	
	TOptional<FGameplayAttribute> FindAttribute(const FGameplayAttribute& AttributeToFind);
	
	TMap<TObjectPtr<UObject>, TArray<FDelegateHandleWrapper>> AttributeBindHandlesOfSource;
	TMap<FDelegateHandle,FGameplayAttribute> AttributeBindHandles;
};
