// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Vehicle.h"
#include "Components/ActorComponent.h"
#include "Input/CharonInputComponent.h"
#include "VehicleAIComponent.generated.h"

struct FGameplayTag;
struct FInputActionValue;
class ACharonAIController;
class AVehicle;

//DECLARE_MULTICAST_DELEGATE_FiveParams(FOnRiderOrderedToAI, const FInputActionValue& ActionValue, FGameplayTag InputTag, bool bIsAbilityAction, bool bIsButtonPressed, bool bNeedServerRPC);


/* 
 * 베히클 관련 AI 캐릭터 생성을 맡을 컴포넌트. 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTCHARON_API UVehicleAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UVehicleAIComponent();
	
	virtual void BeginPlay() override;
	template<class UserClass, typename CallbackFuncType>
	void RegisterAIInputReceiver(UserClass* Object, CallbackFuncType CallbackFunc);
	
	// 플레이어가 특정 좌석의 AI 캐릭터에게 입력을 보내는 함수
	UFUNCTION(BlueprintCallable)
	void PushAIInput(ACharacter* Rider, int32 DestinationSeat, const FInputActionValue& InputActionValue, FGameplayTag InputTag, bool bIsAbilityAction, bool bIsButtonPressed);
	
	static UVehicleAIComponent* FindVehicleAIComponent(const ACharacter* Rider);
	
protected:
	

	virtual void OnRegister() override;
	
	void SpawnAIRiders();
	void ForceAIRide(ACharacter* Rider);
	
	int32 FindRiderIdx(AActor* Rider) const;
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_PushAIInput(ACharacter* Rider, int32 DestinationSeat, const FInputActionValue& InputActionValue, FGameplayTag InputTag, bool bIsAbilityAction, bool bIsButtonPressed);
	
	TArray<TObjectPtr<ACharacter>> AIRiders;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AVehicle> OwnerVehicle;
	
	// 일단 임시로 스폰할 AI캐릭터, AI컨트롤러 클래스는 수동지정.
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> AICharacterClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharonAIController> AIControllerClass;
	
	
	TArray<FOnInputActionTriggered> InputOrderDelegates;
	
public:
	
};

template <class UserClass, typename CallbackFuncType>
void UVehicleAIComponent::RegisterAIInputReceiver(UserClass* Object, CallbackFuncType CallbackFunc)
{
	static_assert(TIsDerivedFrom<UserClass, UActorComponent>::IsDerived,
		"Object must be derived from UActorComponent");
	
	if (AActor* Owner = Object->GetOwner())
	{
		const int32 RiderIdx = FindRiderIdx(Owner);
		if (RiderIdx >= 0)
		{
			check(RiderIdx < InputOrderDelegates.Num())
			
			InputOrderDelegates[RiderIdx].AddUObject(Object, CallbackFunc);
		}
	}	
}
