// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/CharonAbilitySet.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "AbilityGrantor.generated.h"

class UCharonAbilitySet;

UCLASS(Blueprintable)
class PROJECTCHARON_API AAbilityGrantor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAbilityGrantor();

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCharonAbilitySet> AbilitiesToGrant;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UPROPERTY()
	TMap<UCharonAbilitySystemComponent*, FCharonAbilitySet_GrantedHandles> GrantedHandlesMap;
	
private :
	
	
	UPROPERTY(VisibleDefaultsOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UBoxComponent> OverlapBox;
};
