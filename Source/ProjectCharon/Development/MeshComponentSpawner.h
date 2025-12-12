// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshComponentSpawner.generated.h"

UCLASS()
class PROJECTCHARON_API AMeshComponentSpawner : public AActor
{
	GENERATED_BODY()

public:
	
	AMeshComponentSpawner();

protected:


public:
	UFUNCTION(BlueprintCallable)
	void SpawnMeshComponent(UStaticMesh* MeshToSpawn);

	UFUNCTION(BlueprintCallable)
	void AttachMeshes();

	UFUNCTION(BlueprintCallable)
	void DetachMeshes();
	
	UFUNCTION(BlueprintCallable)
	void SpawnMeshAndAttachToTarget(AActor* TargetActor, UStaticMesh* MeshToSpawn, FName SocketNameToAttach = FName("None"));

	UFUNCTION(BlueprintCallable)
	void SpawnSkeletalMeshAndAttachToTarget(AActor* TargetActor, USkeletalMesh* MeshToSpawn, FName SocketNameToAttach = FName("None"));
	
	UPROPERTY(BlueprintReadWrite)
	TArray<UStaticMeshComponent*> Meshes;

	
};
