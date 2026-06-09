// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "BakedShallowWaterSimulationComponent.h"
#include "Landscape.h"
#include "Math/Float16Color.h"
#include "NiagaraSystem.h"
#include "UObject/GCObject.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "ShallowWaterRiverActor.generated.h"

#define UE_API WATERADVANCED_API

class UNiagaraComponent;
class UNiagaraSystem;
class AWaterBody;
class AWaterZone;
class ALandscape;

UENUM(BlueprintType)
enum EShallowWaterRenderState : int
{
	WaterComponent,
	WaterComponentWithBakedSim,
	LiveSim,
	BakedSim,
	DebugRenderBottomContour,
	DebugRenderFoam,
};

///////////////
USTRUCT()
struct FShallowWaterChunk
{
	GENERATED_BODY()

	// --------------------------------------------------------
	// 1. 공간 및 인덱스 데이터
	// --------------------------------------------------------
	UPROPERTY()
	FIntPoint GridIndex; // 전체 그리드 상에서의 (X, Y) 인덱스 (이웃 탐색용)

	UPROPERTY()
	FVector SystemPos; // 이 청크의 월드 기준점 (나이아가라 시스템 위치)

	UPROPERTY()
	FVector2D ChunkWorldSize; // 이 청크의 실제 월드 크기 (마진/오버랩이 적용된 크기)
	
	// 🚨 CPU 물리 연산용: 마진이 잘려나간 순수 알맹이 데이터
	UPROPERTY()
	TArray<FVector4> BakedPhysicsData;

	UPROPERTY()
	int32 BaseResX = 0;
	
	UPROPERTY()
	int32 BaseResY = 0;
	

	// --------------------------------------------------------
	// 2. 나이아가라 시뮬레이션 코어
	// --------------------------------------------------------
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> RiverSimSystem;

	// --------------------------------------------------------
	// 3. 렌더 타깃 (시뮬레이션 결과물)
	// 기존의 단일 RT들을 청크별로 분리
	// --------------------------------------------------------
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> SimGridRT; // (기존 BakedWaterSurfaceRT)

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> FoamRT;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> NormalRT;

	UPROPERTY()
	TObjectPtr<UTexture2D> BakedWaterSurfaceTexture;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> BakedFoamTexture;

	UPROPERTY()
	TObjectPtr<UTexture2D> BakedWaterSurfaceNormalTexture;
	
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> NormalDetailRT;
	
	// --------------------------------------------------------
	// 4. 상태 관리 (최적화 및 트랜지션)
	// --------------------------------------------------------
	UPROPERTY()
	bool bIsActive; // 현재 라이브 시뮬레이션이 돌아가고 있는지 여부

	UPROPERTY()
	bool bNeedsColdStart; // 비활성화 상태에서 방금 켜져서 초기화(Baked 데이터 주입 등)가 필요한지 여부

	FShallowWaterChunk()
		: GridIndex(ForceInitToZero)
		, SystemPos(ForceInitToZero)
		, ChunkWorldSize(ForceInitToZero)
		, RiverSimSystem(nullptr)
		, SimGridRT(nullptr)
		, FoamRT(nullptr)
		, NormalRT(nullptr)
		, bIsActive(false)
		, bNeedsColdStart(true)
	{}
};


UCLASS(MinimalAPI, BlueprintType, HideCategories = (Physics, Replication, Input, Collision))
class UShallowWaterRiverComponent : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Niagara River Simulation"))
	TObjectPtr <class UNiagaraSystem> NiagaraRiverSimulation;

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Resolution Max Axis"))
	int ResolutionMaxAxis;

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Source Width"))
	float SourceSize;

	// (추가) 청크 분할 개수를 에디터에서 설정 (예: X:2, Y:2)
	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Chunk Grid Dimensions"))
	FIntPoint ChunkGridDimensions; 
	
	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Speed"))
	float SimSpeed = 10.f;

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Num Substeps"))
	int NumSteps = 10;

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Match Spline"))
	bool bMatchSpline = true;

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Match Spline Height Amount", EditCondition = "bMatchSpline"))
	float MatchSplineHeightAmount = 2.;

	UPROPERTY(EditAnywhere, Category = "Simulation", meta = (DisplayName = "Remove Outside Spline Amount", EditCondition = "bMatchSpline"))
	float RemoveOutsideSplineAmount = 50.;

	UPROPERTY(EditAnywhere, Category = "Simulation")
	bool bDisableCPUThrottling = true;

	UPROPERTY(EditAnywhere, Category = "Water", meta = (DisplayName = "Source River Water Bodies"))
	TArray<TSoftObjectPtr<AWaterBody>> SourceRiverWaterBodies;

	UPROPERTY(EditAnywhere, Category = "Water", meta = (DisplayName = "Sink River Water Bodies"))
	TArray<TSoftObjectPtr<AWaterBody>> SinkRiverWaterBodies;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Render State"))
	TEnumAsByte<EShallowWaterRenderState> RenderState = EShallowWaterRenderState::WaterComponent;
	
	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Baked Sim Material"))
	TObjectPtr <class UMaterialInstance> BakedSimMaterial;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Baked Sim River To Lake Transition Material"))
	TObjectPtr <class UMaterialInstance> BakedSimRiverToLakeTransitionMaterial;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Baked Sim River To Ocean Transition Material"))
	TObjectPtr <class UMaterialInstance> BakedSimRiverToOceanTransitionMaterial;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Spline River Material"))
	TObjectPtr <class UMaterialInstance> SplineRiverMaterial;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Spline River To Lake Transition Material"))
	TObjectPtr <class UMaterialInstance> SplineRiverToLakeTransitionMaterial;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Spline River To Ocean Transition Material"))
	TObjectPtr <class UMaterialInstance> SplineRiverToOceanTransitionMaterial;

	////////////나중에 지워
	UPROPERTY(EditAnywhere, Category = "Rendering")
	TObjectPtr <class UMaterialInstance> MyTestLakeMaterial;

	///~~~~~~~~나중에 지워
	
	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Smoothing Width"))
	int SmoothingWidth = 5;

	UPROPERTY(EditAnywhere, Category = "Rendering", meta = (DisplayName = "Smoothing Cutoff"))
	float SmoothingCutoff = 500.f;

	UPROPERTY()
	bool bUseVirtualTextures = true;

	UPROPERTY(VisibleAnywhere, Category = "MyTest")
	FVector2D SimRes;

	UPROPERTY()
	float  SimDx;

	UPROPERTY(VisibleAnywhere, Category = "MyTest")
	TObjectPtr<UTexture2D> BakedWaterSurfaceTexture;
	
	UPROPERTY()
	TObjectPtr<UTexture2D> BakedFoamTexture;

	UPROPERTY()
	TObjectPtr<UTexture2D> BakedWaterSurfaceNormalTexture;

	//UPROPERTY(EditAnywhere, Category = "Shallow Water")
	//TObjectPtr<UTexture2D> SignedDistanceToSplineTexture;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	bool bUseCapture = true;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	bool bRecursivelyAddAttachedActors = false;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	TArray<TSoftObjectPtr<AActor>> BottomContourLandscapeActors;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	TArray<TSoftObjectPtr<AActor>> BottomContourActors;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	TArray<FName> BottomContourTags;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	float BottomContourCaptureOffset = 15000.f;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	TArray<TSoftObjectPtr<AActor>> DilatedBottomContourActors;

	UPROPERTY(EditAnywhere, Category = "Collisions")
	TArray<FName> DilatedBottomContourTags;		

	UPROPERTY(EditAnywhere, Category = "Collisions")
	float BottomContourCollisionDilation = 0.f;

	UE_API virtual void PostLoad() override;

	UE_API virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UE_API virtual void BeginPlay() override;

	UE_API virtual void OnUnregister() override;

	UE_API void UpdateRenderState();

	UE_API void SetWaterMIDParameters(UMaterialInstanceDynamic* WaterMID);

#if WITH_EDITOR
	UE_API void Rebuild();

	UE_API void AddActorsToRawArray(const TArray<TSoftObjectPtr<AActor>>& ActorsArray, TArray<AActor*>& BottomContourActorsRawPtr);

	UE_API void AddTaggedActorsToArray(TArray<FName>& TagsToUse, TArray<AActor*>& BottomContourActorsRawPtr);
	
	void Bake();

	UE_API void InitializeVirtualTexture(TObjectPtr<UTexture2D> InTexture);

	UE_API virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UFUNCTION()
	UE_API void OnWaterInfoTextureArrayCreated(const UTextureRenderTarget2DArray* InWaterInfoTexture);

	bool ShouldDisableCPUThrottling() { return true; }
#endif // WITH_EDITOR

	UFUNCTION(BlueprintCallable, Category = "Shallow Water River")
	UE_API void SetPaused(bool Pause);

protected:
	// Asset can be set in Project Settings - Plugins - Water ShallowWaterSimulation
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Shallow Water")
	TObjectPtr<UNiagaraComponent> RiverSimSystem;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Shallow Water")
	TObjectPtr<const UTextureRenderTarget2DArray> WaterInfoTexture;

	// 바뀐 시스템에서 RT 애들은 안씀. 문제가 될 수도 있으니 일단 남겨놓음. 
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Shallow Water")
	TObjectPtr<UTextureRenderTarget2D> BakedWaterSurfaceRT;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Shallow Water")
	TObjectPtr<UTextureRenderTarget2D> BakedFoamRT;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Shallow Water")
	TObjectPtr<UTextureRenderTarget2D> BakedWaterSurfaceNormalRT;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "Shallow Water")
	TObjectPtr<UBakedShallowWaterSimulationComponent> BakedSim;

	UE_API bool QueryWaterAtSplinePoint(TSoftObjectPtr<AWaterBody> WaterBody, int SplinePoint, FVector& OutPos, FVector& OutTangent, float& OutWidth, float& OutDepth);
	UE_API void ConvertToVirtualTextures();

private:
	bool bIsInitialized;	
	bool bTickInitialize;
	bool bRenderStateTickInitialize;

	UPROPERTY()
	TSet <TSoftObjectPtr<AWaterBody>> AllWaterBodies;

	UPROPERTY(VisibleAnywhere, Category = "MyTest")
	FVector2D WorldGridSize;

	UPROPERTY(VisibleAnywhere, Category = "MyTest")
	FVector SystemPos;
	
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> NormalDetailRT;

	FDelegateHandle ShouldDisableCPUThrottlingDelegateHandle;

	// Find/create the level set renderer singleton actor as required. Return whether the found or created actor.
	TObjectPtr<UTextureRenderTarget2D> GetSharedFFTOceanPatchNormalRTFromSubsystem(UWorld* World);

	FBoxSphereBounds InitializeCaptureDI(const FName &DIName, TArray<AActor*> RawActorPtrArray);
	
////////// 내가 수정한 것들
public:

	//////////// watersurfacetexture 테스트 중
	UPROPERTY(EditAnywhere, Category = "MyTest")
	AActor* MyTestWaterSurfaceActor;
	//////////// watersurfacetexture 테스트 끝
protected:
	
	// UPROPERTY(EditAnywhere, Category = "MyTest")
	// bool bDoesTest = false;
	
	// UPROPERTY(EditAnywhere, Category = "MyTest")
	// int TestIndex = 0;
	
	UPROPERTY()
	TArray<FShallowWaterChunk> ShallowWaterChunks;
	
	UPROPERTY(EditAnywhere, Category = "MyTest")
	int32 MarginCells = 15;
	
	UPROPERTY(VisibleAnywhere, Category = "MyTest")
	float AlignedOverlapMargin;
	
	UPROPERTY()
	FVector2D BaseChunkSize;
	
	UPROPERTY()
	FVector2D BaseChunkRes;
	
	// UPROPERTY()
	// float ExpectedSimDx;
	
	// 테스트용
	UPROPERTY(EditAnywhere, Category = "MyTest")
	UTextureRenderTarget2D* BakedWaterSurfaceRTForCheck;
	
	FBoxSphereBounds InitializeCaptureDI(UNiagaraComponent* TargetSimSystem, const FName &DIName, TArray<AActor*> RawActorPtrArray);
	
	
	
private:
	
};

UCLASS(MinimalAPI, BlueprintType, HideCategories = (Physics, Replication, Input, Collision))
class AShallowWaterRiver : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	// Asset can be set in Project Settings - Plugins - Water ShallowWaterSimulation
	UPROPERTY(VisibleAnywhere, Category = "Shallow Water")
	TObjectPtr<UShallowWaterRiverComponent> ShallowWaterRiverComponent;	
};

#undef UE_API
