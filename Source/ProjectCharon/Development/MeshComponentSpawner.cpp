// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshComponentSpawner.h"


// Sets default values
AMeshComponentSpawner::AMeshComponentSpawner()
{
}

void AMeshComponentSpawner::SpawnMeshComponent(UStaticMesh* MeshToSpawn)
{
	UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("StaticMesh"));
	// 3. 컴포넌트 설정
	NewMeshComponent->SetStaticMesh(MeshToSpawn);
	// 필요에 따라 콜리전, 렌더링 등 다른 설정을 합니다.
	NewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Meshes.Add(NewMeshComponent);
}

void AMeshComponentSpawner::AttachMeshes()
{
	for(UStaticMeshComponent* Mesh : Meshes)
	{
		
		Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
		Mesh->RegisterComponent();
	}
}

void AMeshComponentSpawner::DetachMeshes()
{
	for(UStaticMeshComponent* Mesh : Meshes)
	{
		if(Mesh->GetOwner() == this)
		{
			Mesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		}
	}
}

void AMeshComponentSpawner::SpawnMeshAndAttachToTarget(AActor* TargetActor, UStaticMesh* MeshToSpawn,
                                                       FName SocketNameToAttach)
{
	if (!TargetActor || !MeshToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachMeshToActor: TargetActor or MeshToSpawn is invalid."));
		return;
	}

	// 1. 스태틱 메시 컴포넌트 동적 생성
	// NewObject 함수를 사용합니다. 첫 번째 인자는 Outer(소유자)이며, 보통 컴포넌트를 생성하는 액터 자신(this)이나
	// 부착될 대상 액터(TargetActor)를 지정합니다.
	UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(TargetActor, UStaticMeshComponent::StaticClass(), TEXT("AttachedMesh"));

	if (!NewMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AttachMeshToActor: Failed to create NewMeshComponent."));
		return;
	}

	// 2. 컴포넌트 등록
	// NewObject로 생성된 컴포넌트는 반드시 RegisterComponent()를 호출해야 월드에 나타나고 활성화됩니다.
	NewMeshComponent->RegisterComponent();

	// 3. 컴포넌트 설정
	NewMeshComponent->SetStaticMesh(MeshToSpawn);
	// 필요에 따라 콜리전, 렌더링 등 다른 설정을 합니다.
	NewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 4. 타겟 액터에 부착
	// AttachToComponent 함수를 사용합니다.
	// 첫 번째 인자는 부착될 부모 컴포넌트입니다. 보통 대상 액터의 루트 컴포넌트나 메시 컴포넌트를 지정합니다.
	USceneComponent* AttachParent = TargetActor->GetRootComponent();
    
	// 부착 규칙을 설정합니다.
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	NewMeshComponent->AttachToComponent(AttachParent, AttachmentRules, SocketNameToAttach);

	// 이제 NewMeshComponent는 TargetActor의 자식이 되어 함께 움직입니다.
	UE_LOG(LogTemp, Log, TEXT("Successfully created and attached mesh component."));
}

void AMeshComponentSpawner::SpawnSkeletalMeshAndAttachToTarget(AActor* TargetActor, USkeletalMesh* MeshToSpawn,
	FName SocketNameToAttach)
{
	if (!TargetActor || !MeshToSpawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachMeshToActor: TargetActor or MeshToSpawn is invalid."));
		return;
	}

	// 1. 스태틱 메시 컴포넌트 동적 생성
	// NewObject 함수를 사용합니다. 첫 번째 인자는 Outer(소유자)이며, 보통 컴포넌트를 생성하는 액터 자신(this)이나
	// 부착될 대상 액터(TargetActor)를 지정합니다.
	USkeletalMeshComponent* NewMeshComponent = NewObject<USkeletalMeshComponent>(TargetActor, USkeletalMeshComponent::StaticClass(), TEXT("AttachedMesh"));

	if (!NewMeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AttachMeshToActor: Failed to create NewMeshComponent."));
		return;
	}

	// 2. 컴포넌트 등록
	// NewObject로 생성된 컴포넌트는 반드시 RegisterComponent()를 호출해야 월드에 나타나고 활성화됩니다.
	NewMeshComponent->RegisterComponent();

	// 3. 컴포넌트 설정
	NewMeshComponent->SetSkeletalMesh(MeshToSpawn);
	// 필요에 따라 콜리전, 렌더링 등 다른 설정을 합니다.
	NewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 4. 타겟 액터에 부착
	// AttachToComponent 함수를 사용합니다.
	// 첫 번째 인자는 부착될 부모 컴포넌트입니다. 보통 대상 액터의 루트 컴포넌트나 메시 컴포넌트를 지정합니다.
	USceneComponent* AttachParent = TargetActor->GetRootComponent();
    
	// 부착 규칙을 설정합니다.
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	NewMeshComponent->AttachToComponent(AttachParent, AttachmentRules, SocketNameToAttach);

	// 이제 NewMeshComponent는 TargetActor의 자식이 되어 함께 움직입니다.
	UE_LOG(LogTemp, Log, TEXT("Successfully created and attached mesh component."));
}

