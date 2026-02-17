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

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	// 어트리뷰트 값 변화 델리게이트에 바인딩/언바인딩 하기. EventSource - 델리게이트에 바인드할때 얘 이름으로. EventSource 파괴되면 바인딩도 해제되게.
	// TODO : 나중에 Bind/UnBind는 BlueprintCallable을 없애고 다른 c++클래스 경유해서 하게 하는게 나을듯?  
	UFUNCTION(BlueprintCallable, Category = "Charon | Ability", DisplayName="BindEventOnAttributeChange")
	FDelegateHandleWrapper K2_BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged_Dynamic Event);
	FDelegateHandleWrapper BindEventOnAttributeChange(UObject* EventSource, FGameplayAttribute AttributeToBind, FOnCharonAttributeChanged Event);
	
	UFUNCTION(BlueprintCallable, Category = "Charon | Ability", DisplayName="UnBindEventOnAttributeChange")
	void K2_UnBindEventOnAttributeChange(const FDelegateHandleWrapper EventHandle);
	void UnBindEventOnAttributeChange(const FDelegateHandleWrapper EventHandle);
	
	void TryActivateAbilitiesOnSpawn();
	
	//void UnBindEventOnAttributeChange(FDelegateHandle EventHandle);
	

	///////////////////테스트용
	UFUNCTION(BlueprintCallable, meta=(DevelopmentOnly))
	void CheckAttributeBinds();

	UFUNCTION(BlueprintCallable, meta=(DevelopmentOnly))
	void CheckAbilitySpecs(FGameplayAbilitySpecHandle Handle);
	////////////////////
	
	// Input
	void AbilityLocalInputTagPressed(FGameplayTag InputTag);
	void AbilityLocalInputTagReleased(FGameplayTag InputTag);

protected:

	// 테스트를 위해 추가, 언제든 삭제 가능.
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	// 데이터 테이블로 어트리뷰트 셋 초기화.(기존 ASC는 OnRegister에서 이를 수행하더라)
	void InitAttributesWithDefaultData();
	
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	
	TOptional<FGameplayAttribute> FindAttribute(const FGameplayAttribute& AttributeToFind);
	
	TMap<TObjectPtr<UObject>, TArray<FDelegateHandleWrapper>> AttributeBindHandlesOfSource;
	TMap<FDelegateHandle,FGameplayAttribute> AttributeBindHandles;
};
