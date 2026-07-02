// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShallowWaterRiverActor.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

#include "WaterBodyActor.h"
#include "WaterBodyRiverComponent.h"
#include "WaterSplineComponent.h"
#include "WaterSubsystem.h"

#include "BakedShallowWaterSimulationComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SplineMeshComponent.h"
#include "PhysicsEngine/ConvexElem.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/OverlapResult.h"

#include "TextureResource.h"
#include "ShallowWaterCommon.h"
#include "FFTOceanPatchSubsystem.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Math/Float16Color.h"
#include "Landscape.h"
#include "LandscapeStreamingProxy.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "LevelInstance/LevelInstanceSubsystem.h"

#include "EngineUtils.h"

#if WITH_EDITOR
#include "Editor.h"
#endif


#include "WaterBodyLakeActor.h"
#include "WaterBodyLakeComponent.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ShallowWaterRiverActor)

static int DirX[] = {0,1,0,-1,0};
static int DirY[] = {0,0,1,0,-1};

bool bShallowWaterRiverDebugVisualize = false;
FAutoConsoleVariableRef CVarShallowWaterRiverDebugVisualize(TEXT("r.ShallowWater.RiverDebugVisualize"), bShallowWaterRiverDebugVisualize, TEXT(""));

UShallowWaterRiverComponent::UShallowWaterRiverComponent(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;	

#if WITH_EDITORONLY_DATA
	bTickInEditor = true;
#endif // WITH_EDITORONLY_DATA

	bIsInitialized = false;
	bTickInitialize = false;
	bRenderStateTickInitialize = false;

	ResolutionMaxAxis = 512;
	SourceSize = 1000;
	ChunkGridDimensions = {1,1};
	
	// initialize landscape array with all landscapes
	if (GetWorld())
	{
		for (TActorIterator<ALandscape> It(GetWorld(), ALandscape::StaticClass()); It; ++It)
		{
			BottomContourLandscapeActors.Add(*It);
		}

		for (TActorIterator<ALandscapeStreamingProxy> It(GetWorld(), ALandscapeStreamingProxy::StaticClass()); It; ++It)
		{
			BottomContourLandscapeActors.Add(*It);
		}
	}
}

TObjectPtr<UTextureRenderTarget2D> UShallowWaterRiverComponent::GetSharedFFTOceanPatchNormalRTFromSubsystem(UWorld* World)
{
	if (World != nullptr)
	{
		UFFTOceanPatchSubsystem *OceanPatchSubsystem = World->GetSubsystem<UFFTOceanPatchSubsystem>();

		if (OceanPatchSubsystem != nullptr)
		{
			return OceanPatchSubsystem->GetOceanNormalRT(World);
		}
		else
		{
			UE_LOG(LogShallowWater, Warning, TEXT("No valid FFT ocean patch subsystem."));	
		}
	}
	else
	{
		UE_LOG(LogShallowWater, Warning, TEXT("No valid World."));
	}

	return nullptr;
}

FBoxSphereBounds UShallowWaterRiverComponent::InitializeCaptureDI(const FName& DIName, TArray<AActor*> RawActorPtrArray)
{
	UNiagaraFunctionLibrary::SetSceneCapture2DDataInterfaceManagedMode(RiverSimSystem, DIName,
				ESceneCaptureSource::SCS_SceneDepth,
				FIntPoint(ResolutionMaxAxis, ResolutionMaxAxis),
				ETextureRenderTargetFormat::RTF_R32f,
				ECameraProjectionMode::Orthographic,
				90.0f,
				FMath::Max(WorldGridSize.X, WorldGridSize.Y),
				true,
				false,
				RawActorPtrArray);

	// accumulate bounding box for river water bodies
	FBoxSphereBounds::Builder BottomContourCombinedWorldBoundsBuilder;
	for (AActor *BottomContourActor : RawActorPtrArray)
	{
		if (BottomContourActor != nullptr)
		{
			// accumulate bounds
			FBoxSphereBounds WorldBounds;
			BottomContourActor->GetActorBounds(false, WorldBounds.Origin, WorldBounds.BoxExtent);

			BottomContourCombinedWorldBoundsBuilder += WorldBounds;
		}
		else
		{
			UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - skipping null bottom contour boundary actor found"));
			continue;
		}
	}
	return FBoxSphereBounds(BottomContourCombinedWorldBoundsBuilder);	
}

void UShallowWaterRiverComponent::InitSequentialSimulation(bool bIsFirstPass)
{
	if (ShallowWaterChunks.IsEmpty() || RenderState != EShallowWaterRenderState::LiveSim)
	{
		return;
	}
	
	BakingQueue.Empty();
	
	if(bIsFirstPass)
	{
		CurrentBakePass = 0;
		CurrentTargetChunkIndex = -1;
		// 거리, 검사 여부 배열 초기화
		ChunkDistanceFromSource.Empty();
		ChunkBeenSimulated.Empty();
		ChunkDistanceFromSource.AddZeroed(ShallowWaterChunks.Num());
		ChunkBeenSimulated.AddZeroed(ShallowWaterChunks.Num());
		SourceChunkIndices.Empty();
		
		for (int32 i = 0; i < ShallowWaterChunks.Num(); ++i)
		{
			ChunkDistanceFromSource[i] = 9999999; // 초기화
			ChunkBeenSimulated[i] = false;

			if (IsChunkOverlappingSource(ShallowWaterChunks[i]))
			{
				SourceChunkIndices.Add(i);
				ChunkDistanceFromSource[i] = 0;
			}
		}

		// 2. 전체 청크에 대해 'Source까지의 그리드 거리' 계산 (맨해튼 거리)
		for (int32 i = 0; i < ShallowWaterChunks.Num(); ++i)
		{
			for (int32 SourceIdx : SourceChunkIndices)
			{
				int32 DistX = FMath::Abs(ShallowWaterChunks[i].GridIndex.X - ShallowWaterChunks[SourceIdx].GridIndex.X);
				int32 DistY = FMath::Abs(ShallowWaterChunks[i].GridIndex.Y - ShallowWaterChunks[SourceIdx].GridIndex.Y);

				ChunkDistanceFromSource[i] = FMath::Min(ChunkDistanceFromSource[i], DistX + DistY);
			}
		}
	}	
	
	// 3. 소스 청크들을 큐에 넣고 첫 시작 준비
	for (int32 SourceIdx : SourceChunkIndices)
	{
		BakingQueue.Add(SourceIdx);
	}
	
	bIsSequentialBaking = true;
	CurrentBakingFrameCount = 0;
	
}

void UShallowWaterRiverComponent::TickSimulate()
{
	if (!bIsSequentialBaking || ShallowWaterChunks.IsEmpty() || RenderState != EShallowWaterRenderState::LiveSim)
	{
		return;
	}
	
	CurrentBakingFrameCount++;
	
	while (!BakingQueue.IsEmpty() && ChunkBeenSimulated[BakingQueue[BakingQueue.Num() - 1]])
	{
		BakingQueue.RemoveAt(BakingQueue.Num() - 1);
	}
	
	if (CurrentTargetChunkIndex < 0 && BakingQueue.IsEmpty())
	{
		CurrentBakePass++;
		if (CurrentBakePass >= SimulationRepetitionNum)
		{
			// 모든 패스가 끝남! 시뮬레이션 최종 종료
			bIsSequentialBaking = false;
			UE_LOG(LogTemp, Warning, TEXT("Sequential Baking Finished!"));
			// 여기서 Bake() 호출 혹은 사용자 수동 Bake 대기
			return;
		}
		// 큐가 비었지만 아직 패스가 남았다면? 초기화 후 다시 시작
		for (int32 i = 0; i < ChunkBeenSimulated.Num(); ++i)
		{
			ChunkBeenSimulated[i] = false;
		}
		InitSequentialSimulation(false); // Source 다시 큐에 넣고 상태 리셋
		return;
	}

	// 시간 다 지났거나 현재 타겟이 없을 때
	if (CurrentTargetChunkIndex < 0 || CurrentBakingFrameCount > FramesForChunkSimulation)
	{
		if (CurrentTargetChunkIndex >= 0)
		{
			// 경계면 물 검사 및 이웃 큐 추가
			CheckBoundariesAndQueueNeighbors(CurrentTargetChunkIndex);	
			// 타깃과 이웃 시스템 끄기 (일시정지)
			ActivateChunkAndNeighbors(CurrentTargetChunkIndex, false);
		}
		
		// 🚨 큐 정렬: Source까지의 거리가 가장 가까운 놈이 최우선! (내림차순, 배열은 뒤에 있는게 빼기 좋으니깐.)
		BakingQueue.Sort([this](const int32& A, const int32& B)
		{
			return ChunkDistanceFromSource[A] > ChunkDistanceFromSource[B];
		});
		
		// 큐의 맨 앞(최우선 순위) 청크 꺼내기
		
		CurrentTargetChunkIndex = BakingQueue.IsEmpty() ? -1 : BakingQueue[BakingQueue.Num() - 1];

		FString Str = "";
		for (int32 ChunkIndex : BakingQueue)
		{
			Str += FString::Printf(TEXT("%d "), ChunkIndex);
		}
		
		if (CurrentTargetChunkIndex >= 0)
		{
			ChunkBeenSimulated[CurrentTargetChunkIndex] = true;
			BakingQueue.RemoveAt(BakingQueue.Num() - 1);
			
			ActivateChunkAndNeighbors(CurrentTargetChunkIndex, true);
		}
		
		CurrentBakingFrameCount = 0;

		//UpdateChunkSimStates(CurrentTargetChunkIndex);
	}
	
}

// void UShallowWaterRiverComponent::UpdateChunkSimStates(int32 CenterChunkIndex)
// {
// 	TSet<int32> ActiveSet;
//
// 	// 1. 이번 턴에 연산이 돌아가야 할 청크(Center + 4방향 이웃) 목록 수집
// 	if (CenterChunkIndex >= 0)
// 	{
// 		FShallowWaterChunk& CenterChunk = ShallowWaterChunks[CenterChunkIndex];
// 		int32 cx = CenterChunk.GridIndex.X;
// 		int32 cy = CenterChunk.GridIndex.Y;
//
// 		for (int i = 0; i < 5; i++) 
// 		{
// 			int nx = cx + DirX[i];
// 			int ny = cy + DirY[i];
//
// 			if (nx < 0 || nx >= ChunkGridDimensions.X || ny < 0 || ny >= ChunkGridDimensions.Y)
// 			{
// 				continue;
// 			}
//
// 			int32 ChunkIdx = ny * ChunkGridDimensions.X + nx;
// 			if (ChunkIdx < ShallowWaterChunks.Num())
// 			{
// 				ActiveSet.Add(ChunkIdx);
// 			}
// 		}
// 	}
//
// 	// 2. 파라미터 토글 (연산 및 거품 제어)
// 	for (int32 i = 0; i < ShallowWaterChunks.Num(); ++i)
// 	{
// 		if (!ShallowWaterChunks[i].RiverSimSystem)
// 		{
// 			continue;
// 		}
// 		
// 		// 이 청크가 이번에 연산을 해야 하는 그룹(타깃 or 이웃)인가?
// 		bool bShouldSimulate = ActiveSet.Contains(i);
// 		// 연산을 해야 하면 Cached 연산을 끄고(false), 쉬어야 하면 켬(true)
// 		ShallowWaterChunks[i].RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), !bShouldSimulate);
// 		// 거품 생성 여부도 동일하게 제어
// 		ShallowWaterChunks[i].RiverSimSystem->SetVariableBool(FName("GenerateFoam"), bShouldSimulate);
//
// 		// 디버깅 로그 (필요 시 주석 처리)
// 		if (bShouldSimulate)
// 		{
// 			ShallowWaterChunks[i].RiverSimSystem->ReinitializeSystem();
// 			
// 			UE_LOG(LogTemp, Warning, TEXT("야돈 [Simulate ON] Chunk:%d"), i);
// 		}
// 	}
// }


void UShallowWaterRiverComponent::ActivateChunkAndNeighbors(int32 CenterChunkIndex, bool bActive)
{
	FShallowWaterChunk& Chunk = ShallowWaterChunks[CenterChunkIndex];
	int32 cx = Chunk.GridIndex.X;
	int32 cy = Chunk.GridIndex.Y;
	
	for (int i = 0; i < 5; i++)
	{
		int nx = cx + DirX[i];
		int ny = cy + DirY[i];
		
		
		if (nx < 0 || nx >= ChunkGridDimensions.X || ny < 0 || ny >= ChunkGridDimensions.Y)
		{
			continue;
		}
		
		int ChunkIdx = ny * ChunkGridDimensions.X + nx;
		
		if (ShallowWaterChunks.Num() > ChunkIdx && ShallowWaterChunks[ChunkIdx].RiverSimSystem)
		{
			if (bActive)
			{
				// 연산을 해야 하면 Cached 연산을 끄고(false), 쉬어야 하면 켬(true)
				ShallowWaterChunks[ChunkIdx].RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), false);
				// // 거품 생성 여부도 동일하게 제어
				// ShallowWaterChunks[ChunkIdx].RiverSimSystem->SetVariableBool(FName("GenerateFoam"), true);
				
				// 테스트 - 중간 저장한 텍스처를 불러옴.
				ShallowWaterChunks[ChunkIdx].RiverSimSystem->SetVariableTexture(FName("BakedSimTexture"), ShallowWaterChunks[ChunkIdx].BakedWaterSurfaceTexture);
				
				ShallowWaterChunks[ChunkIdx].RiverSimSystem->ReinitializeSystem();
				//ShallowWaterChunks[ChunkIdx].RiverSimSystem->Activate();
			}
			else
			{
							
				// 중간 저장을 위해 텍스처 형식으로 구움.
				ShallowWaterChunks[ChunkIdx].BakedWaterSurfaceTexture = ShallowWaterChunks[ChunkIdx].SimGridRT->
					ConstructTexture2D(this, FString::Printf(TEXT("ChunkBakedTexture%d"), ChunkIdx ), RF_Public);
				
				// if (ChunkIdx == 1)
				// {
				// 	TestVisibleTexture = BakedTexture;
				// 	TestVisibleRT = ShallowWaterChunks[ChunkIdx].SimGridRT;
				// }
				// ////////////////
				
				
				ShallowWaterChunks[ChunkIdx].RiverSimSystem->DeactivateImmediate();
				ShallowWaterChunks[ChunkIdx].RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), true);
			}
		}
	}
	
	
	// /////////
	// for (FShallowWaterChunk& CheckChunk : ShallowWaterChunks)
	// {
	// 	if (CheckChunk.RiverSimSystem->IsActive())
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("야돈 Chunk(%d,%d) is Alive"), CheckChunk.GridIndex.X, CheckChunk.GridIndex.Y); //
	// 	}
	// }
	// /////
	
	// TSet<int32> ActiveSet;
	//
	// // 1. 이번 턴에 연산이 돌아가야 할 청크(Center + 4방향 이웃) 목록 수집
	// if (CenterChunkIndex >= 0)
	// {
	// 	ActiveSet.Add(CenterChunkIndex);
	// 	FShallowWaterChunk& CenterChunk = ShallowWaterChunks[CenterChunkIndex];
	// 	int32 cx = CenterChunk.GridIndex.X;
	// 	int32 cy = CenterChunk.GridIndex.Y;
	//
	// 	for (int i = 0; i < 4; i++) // 4방향 탐색
	// 	{
	// 		int nx = cx + DirX[i];
	// 		int ny = cy + DirY[i];
	//
	// 		if (nx < 0 || nx >= ChunkGridDimensions.X || ny < 0 || ny >= ChunkGridDimensions.Y)
	// 		{
	// 			continue;
	// 		}
	//
	// 		int32 ChunkIdx = ny * ChunkGridDimensions.X + nx;
	// 		if (ChunkIdx < ShallowWaterChunks.Num())
	// 		{
	// 			ActiveSet.Add(ChunkIdx);
	// 		}
	// 	}
	// }
	//
	// // 2. 파라미터 토글 (연산 및 거품 제어)
	// for (int32 i = 0; i < ShallowWaterChunks.Num(); ++i)
	// {
	// 	if (!ShallowWaterChunks[i].RiverSimSystem)
	// 	{
	// 		continue;
	// 	}
	//
	// 	// 이 청크가 이번에 연산을 해야 하는 그룹(타깃 or 이웃)인가?
	// 	bool bShouldSimulate = ActiveSet.Contains(i);
	//
	// 	// 연산을 해야 하면 Cached 연산을 끄고(false), 쉬어야 하면 켬(true)
	// 	ShallowWaterChunks[i].RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), !bShouldSimulate);
 //        
	// 	// 거품 생성 여부도 동일하게 제어
	// 	ShallowWaterChunks[i].RiverSimSystem->SetVariableBool(FName("GenerateFoam"), bShouldSimulate);
	//
	// 	// 디버깅 로그 (필요 시 주석 처리)
	// 	if (bShouldSimulate) UE_LOG(LogTemp, Warning, TEXT("야돈 [Simulate ON] Chunk:%d"), i);
	// }
	
}

void UShallowWaterRiverComponent::CheckBoundariesAndQueueNeighbors(int32 CenterChunkIndex)
{
	FShallowWaterChunk& CenterChunk = ShallowWaterChunks[CenterChunkIndex];
    
    if (!CenterChunk.SimGridRT)
    {
    	UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::CheckBoundariesAndQueueNeighbors, SimGridRT없음 "));
	    return;
    }

    TArray<FFloat16Color> RawPixels;
    CenterChunk.SimGridRT->GameThread_GetRenderTargetResource()->ReadFloat16Pixels(RawPixels);
    
    int32 RTResX = CenterChunk.SimGridRT->SizeX;
    int32 RTResY = CenterChunk.SimGridRT->SizeY;

    // 4방향 물 존재 여부 플래그
    bool bWaterTop = false, bWaterBottom = false, bWaterLeft = false, bWaterRight = false;

	for (int y = MarginCells ; y < RTResY - MarginCells ; y++)
	{
		int x = MarginCells;
		int ReadIdx = (y * RTResX) + x;
		bWaterLeft |= RawPixels.IsValidIndex(ReadIdx) && RawPixels[ReadIdx].G.GetFloat() > 1e-4f;
		
		x = RTResX - MarginCells;
		ReadIdx = (y * RTResX) + x;
		bWaterRight |= RawPixels.IsValidIndex(ReadIdx) && RawPixels[ReadIdx].G.GetFloat() > 1e-4f;
	}
	for (int x = MarginCells ; x < RTResX - MarginCells ; x++)
	{
		int y = MarginCells;
		int ReadIdx = (y * RTResX) + x;
		bWaterBottom |= RawPixels.IsValidIndex(ReadIdx) && RawPixels[ReadIdx].G.GetFloat() > 1e-4f;
		
		y = RTResY - MarginCells;
		ReadIdx = (y * RTResX) + x;
		bWaterTop |= RawPixels.IsValidIndex(ReadIdx) && RawPixels[ReadIdx].G.GetFloat() > 1e-4f;
	}
	
	
    int32 CX = CenterChunk.GridIndex.X;
    int32 CY = CenterChunk.GridIndex.Y;

    // 이웃을 찾아 큐에 넣기 (타깃된 적 없고, 경계에 물이 닿았을 때만)
    auto TryQueueNeighbor = [&](int32 NX, int32 NY, bool bHasWater) 
	{
        if (!bHasWater)
        {
	        return;
        };

        for (int32 i = 0; i < ShallowWaterChunks.Num(); ++i)
        {
            if (ShallowWaterChunks[i].GridIndex.X == NX && ShallowWaterChunks[i].GridIndex.Y == NY)
            {
                // 타깃된 적 없으며, 큐에 중복으로 들어있지 않으면 추가
                if (!ChunkBeenSimulated[i] && !BakingQueue.Contains(i))
                {
                    BakingQueue.Add(i);
                }
                break;
            }
        }
    };

    TryQueueNeighbor(CX, CY + 1, bWaterTop);
    TryQueueNeighbor(CX, CY - 1, bWaterBottom);
    TryQueueNeighbor(CX - 1, CY, bWaterLeft);
    TryQueueNeighbor(CX + 1, CY, bWaterRight);
	
}

bool UShallowWaterRiverComponent::IsChunkOverlappingSource(const FShallowWaterChunk& Chunk)
{
	// 1. Chunk의 2D AABB 정보 설정
    FVector2D ChunkCenter(Chunk.SystemPos.X, Chunk.SystemPos.Y);
    FVector2D ChunkExtents(Chunk.ChunkWorldSize.X * 0.5f, Chunk.ChunkWorldSize.Y * 0.5f);
    FBox2D ChunkBox(ChunkCenter - ChunkExtents, ChunkCenter + ChunkExtents);

    // Chunk의 4개 모서리 + 중심점 (Z는 0으로 무시)
    TArray<FVector> ChunkPoints = {
        FVector(ChunkCenter.X, ChunkCenter.Y, 0), // 중심점
        FVector(ChunkCenter.X - ChunkExtents.X, ChunkCenter.Y - ChunkExtents.Y, 0),
        FVector(ChunkCenter.X + ChunkExtents.X, ChunkCenter.Y - ChunkExtents.Y, 0),
        FVector(ChunkCenter.X + ChunkExtents.X, ChunkCenter.Y + ChunkExtents.Y, 0),
        FVector(ChunkCenter.X - ChunkExtents.X, ChunkCenter.Y + ChunkExtents.Y, 0)
    };

    for (int32 i = 0; i < SourcePosArray.Num(); ++i)
    {
        FVector Pos = SourcePosArray[i];
        FVector Size = FVector(SourceSizeArray[i]);
        float Angle = SourceAngleArray[i];

        // HLSL의 AxisAngleQuat 계산과 완벽히 동일 (Z축 회전)
        FQuat SourceRot = FQuat(FVector(0, 0, 1), Angle);
        FTransform SourceTransform(SourceRot, Pos);

        FVector SourceExtents = Size * 0.5f;

        // 검사 A: 청크의 점들이 Source 박스 내부에 있는지 검사 (HLSL 로직의 C++ 버전)
        bool bOverlaps = false;
        for (const FVector& Pt : ChunkPoints)
        {
            // WorldToLocal 변환 (HLSL의 Inverse Transform과 동일한 기능)
            FVector LocalPt = SourceTransform.InverseTransformPosition(Pt);
            
            // X, Y가 절반 크기(Extents) 안에 들어오면 겹친 것!
            if (FMath::Abs(LocalPt.X) <= SourceExtents.X && 
                FMath::Abs(LocalPt.Y) <= SourceExtents.Y)
            {
                bOverlaps = true;
                break;
            }
        }

        if (bOverlaps) return true;

        // 검사 B: 반대로 Source의 모서리들이 청크(AABB) 내부에 있는지 검사 (소스가 청크보다 작을 때를 대비)
        TArray<FVector> SourceLocalCorners = {
            FVector(-SourceExtents.X, -SourceExtents.Y, 0),
            FVector( SourceExtents.X, -SourceExtents.Y, 0),
            FVector( SourceExtents.X,  SourceExtents.Y, 0),
            FVector(-SourceExtents.X,  SourceExtents.Y, 0)
        };

        for (const FVector& LocalCorner : SourceLocalCorners)
        {
            // LocalToWorld 변환
            FVector WorldCorner = SourceTransform.TransformPosition(LocalCorner);
            if (ChunkBox.IsInside(FVector2D(WorldCorner.X, WorldCorner.Y)))
            {
                return true;
            }
        }
    }

    return false;
}



FBoxSphereBounds UShallowWaterRiverComponent::InitializeCaptureDI(UNiagaraComponent* TargetSimSystem,
                                                                  const FName& DIName, TArray<AActor*> RawActorPtrArray, FIntPoint CaptureResolution, float OrthoWidth)
{
	UNiagaraFunctionLibrary::SetSceneCapture2DDataInterfaceManagedMode(TargetSimSystem, DIName,
				ESceneCaptureSource::SCS_SceneDepth,
				CaptureResolution,
				ETextureRenderTargetFormat::RTF_R32f,
				ECameraProjectionMode::Orthographic,
				90.0f,
				OrthoWidth,
				true,
				false,
				RawActorPtrArray);

	// accumulate bounding box for river water bodies
	FBoxSphereBounds::Builder BottomContourCombinedWorldBoundsBuilder;
	for (AActor *BottomContourActor : RawActorPtrArray)
	{
		if (BottomContourActor != nullptr)
		{
			// accumulate bounds
			FBoxSphereBounds WorldBounds;
			BottomContourActor->GetActorBounds(false, WorldBounds.Origin, WorldBounds.BoxExtent);

			BottomContourCombinedWorldBoundsBuilder += WorldBounds;
		}
		else
		{
			UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - skipping null bottom contour boundary actor found"));
			continue;
		}
	}
	return FBoxSphereBounds(BottomContourCombinedWorldBoundsBuilder);	
}

void UShallowWaterRiverComponent::ConvertToVirtualTextures()
{
#if WITH_EDITOR
	bool HasChanged = false;

	bUseVirtualTextures = true;

	if (BakedWaterSurfaceTexture != NULL && !BakedWaterSurfaceTexture->VirtualTextureStreaming)
	{	
		SimRes = FVector2D(BakedWaterSurfaceTexture->Source.GetSizeX(), BakedWaterSurfaceTexture->Source.GetSizeY());
		RiverSimSystem->SetVariableVec2(FName("SimRes"), SimRes);

		InitializeVirtualTexture(BakedWaterSurfaceTexture);
		UE_LOG(LogShallowWater, Warning, TEXT("Baked water surface texture was not virtual- converting.  Recommended resave."));

		HasChanged = true;
	}
	
	if (BakedWaterSurfaceNormalTexture != NULL && !BakedWaterSurfaceNormalTexture->VirtualTextureStreaming)
	{
		InitializeVirtualTexture(BakedWaterSurfaceNormalTexture);
		UE_LOG(LogShallowWater, Warning, TEXT("Baked water surface normal texture was not virtual- converting.  Recommended resave."));

		HasChanged = true;
	}

	if (BakedFoamTexture != NULL && !BakedFoamTexture->VirtualTextureStreaming)
	{
		InitializeVirtualTexture(BakedFoamTexture);
		UE_LOG(LogShallowWater, Warning, TEXT("Baked foam texture was not virtual- converting.  Recommended resave."));

		HasChanged = true;
	}

	if (HasChanged)
	{
		PostEditChange();
	}
#endif
}

void UShallowWaterRiverComponent::PostLoad()
{
	Super::PostLoad();	

	// ensure all baked textures are virtual for backwards compatibility
	ConvertToVirtualTextures();

	if (RenderState == EShallowWaterRenderState::LiveSim || RiverSimSystem == nullptr)
	{
	#if WITH_EDITOR
		bIsInitialized = false;
		bTickInitialize = false;

		Rebuild();
	#endif
	}
	else
	{
		// RiverSimSystem->ReinitializeSystem();
		// RiverSimSystem->Activate();
		
		
		// // ReInitialize에 activate가 포함되어 있었네?
		// for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
		// {
		// 	Chunk.RiverSimSystem->ReinitializeSystem();
		// 	Chunk.RiverSimSystem->Activate();
		// }
	}

	bRenderStateTickInitialize = false;
}

void UShallowWaterRiverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
#if WITH_EDITOR
	
	// 여기도 수정.
	
	bool bNeedsRebuild = ShallowWaterChunks.Num() == 0;
	
	// lots of tick ordering issues, so we try to initialize on the first tick too
	// if (!bTickInitialize && (RiverSimSystem == nullptr || (RenderState == EShallowWaterRenderState::LiveSim && !bIsInitialized)))
	// {
	// 	bTickInitialize = true;
	// 	Rebuild();
	// }
	if (!bTickInitialize && (bNeedsRebuild || (RenderState == EShallowWaterRenderState::LiveSim && !bIsInitialized)))
	{
		bTickInitialize = true; // 먼저 true로 막아서 중복 진입 방지
		Rebuild();
	}
	else if (bIsInitialized)
	{
		// if (RiverSimSystem)
		// {
		// 	RiverSimSystem->Activate();	
		// }
		// else
		// {
		// 	UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::TickComponent() - null Niagara sim when trying to activate. Please reset."));
		// }
		
		
		if (bIsSequentialBaking)
		{
			TickSimulate();
		}
	}
	else
	{
		// System is in a bad state
		// UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::TickComponent() - null Niagara sim when trying to activate. Please reset."));
	}
#endif

	if (!bRenderStateTickInitialize)
	{
		UpdateRenderState();
	}
}

void UShallowWaterRiverComponent::BeginPlay()
{
	Super::BeginPlay();

	bRenderStateTickInitialize = false;

	UpdateRenderState();

	// make sure the simulation is not going to be run in case of various initialization edge cases
	bool bReadBakedSim = RenderState == EShallowWaterRenderState::BakedSim || RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::WaterComponent;	
	// if (RiverSimSystem != nullptr && bReadBakedSim)
	// {
	// 	RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
	// 	RiverSimSystem->ReinitializeSystem();
	// 	RiverSimSystem->Activate();
	// }
	//
	// // TODO : 여기 뭔가 수정해야될듯.
	// for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	// {
	// 	if (Chunk.RiverSimSystem != nullptr && bReadBakedSim)
	// 	{
	// 		// 수정중~
	// 		//Chunk.RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
	// 		Chunk.RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), true);////
	// 		Chunk.RiverSimSystem->ReinitializeSystem();
	// 		Chunk.RiverSimSystem->Activate();
	// 	}
	// }
	
	
	/////테스트용
	UE_LOG(LogTemp, Warning, TEXT("야도란 Num of WaterBodies : %d"), AllWaterBodies.Num());
	
	
	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
	{
		if (AWaterBody* WaterBody = CurrWaterBody.Get())
		{
			
			
			if (UWaterBodyComponent* CurrWaterBodyComponent =  WaterBody->GetWaterBodyComponent())
			{
				if (UMaterialInstanceDynamic* WaterMID =  CurrWaterBodyComponent->GetWaterMaterialInstance())
				{
					UE_LOG(LogTemp, Warning, TEXT("야도란 WaterMID of %s Is VALID : "), *WaterBody->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("야도란 WaterMID of %s Is NOT!!!! VALID : "), *WaterBody->GetName());
				}
			}
		}
		
	}
	/////
}

void UShallowWaterRiverComponent::OnUnregister()
{
	Super::OnUnregister();
}

#if WITH_EDITOR

void UShallowWaterRiverComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName;
	if (PropertyChangedEvent.Property)
	{
		PropertyName = PropertyChangedEvent.Property->GetFName();
	}
			
	// this should go before rebuild not after...something is wrong
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UShallowWaterRiverComponent, RenderState) && RiverSimSystem != nullptr && RiverSimSystem->IsActive())
	{
		bool bReadBakedSim = RenderState == EShallowWaterRenderState::BakedSim || RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::WaterComponent;	
		RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
	}
	else
	{
		bIsInitialized = false;
		bTickInitialize = false;		
	}

	bRenderStateTickInitialize = false;

	Rebuild();
	UpdateRenderState();
	ReregisterComponent();

}

// void UShallowWaterRiverComponent::Rebuild()
// {	
// 	bIsInitialized = false;
// 	bTickInitialize = false;
//
// 	if (NiagaraRiverSimulation == nullptr)
// 	{
// 		NiagaraRiverSimulation = LoadObject<UNiagaraSystem>(nullptr, TEXT("/WaterAdvanced/Niagara/Systems/Grid2D_SW_River.Grid2D_SW_River"));
// 	}
//
// 	if (RiverSimSystem != nullptr)
// 	{
// 		RiverSimSystem->SetActive(false);
// 		RiverSimSystem->DestroyComponent();
// 		RiverSimSystem = nullptr;
// 	}
// 	
// 	if (ResolutionMaxAxis <= 0)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - resolution must be greater than 0"));
// 		return;
// 	}
//
// 	if (NumSteps <= 0)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - num steps must be greater than 0"));
// 		return;
// 	}
//
// 	if (SimSpeed <= 1e-8)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - speed must be greater than zero"));
// 		return;
// 	}
//
// 	if (NiagaraRiverSimulation == nullptr)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - null Niagara system asset"));
// 		return;
// 	}
//
// 	AllWaterBodies.Empty();
//
// 	// collect all the water bodies	
// 	if (SourceRiverWaterBodies.Num() != 0)
// 	{
// 		for (TSoftObjectPtr<AWaterBody > CurrWaterBody : SourceRiverWaterBodies)
// 		{
// 			if (CurrWaterBody)
// 			{
// 				AllWaterBodies.Add(CurrWaterBody);
// 			}
// 			else 
// 			{
// 				UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - skipping null water body actor found"));
// 				continue;
// 			}
// 		}
// 	}
// 	else	
// 	{
// 		UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - No source water bodies specified"));
// 		return;
// 	}
// 	
// 	if (AllWaterBodies.Num() == 0)
// 	{
// 		UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - No valid source water bodies specified"));
// 		return;
// 	}
//
// 	bool HasValidSinks = false;
// 	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : SinkRiverWaterBodies)
// 	{
// 		if (CurrWaterBody != nullptr)
// 		{
// 			HasValidSinks = true;
// 			AllWaterBodies.Add(CurrWaterBody);			
// 		}
// 		else
// 		{
// 			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - skipping null sink water body actor found"));
// 			continue;
// 		}
// 	}
//
// 	// flush all debug draw lines
// #if ENABLE_DRAW_DEBUG
// 	FlushPersistentDebugLines(GetWorld());
// #endif
//
// 	if (!HasValidSinks)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - no valid sinks, using the first source as a sink"));
// 		SinkRiverWaterBodies.Add(*AllWaterBodies.CreateConstIterator());
// 	}
//
// 	// accumulate bounding box for river water bodies
// 	FBoxSphereBounds::Builder CombinedWorldBoundsBuilder;
// 	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
// 	{
// 		TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = CurrWaterBody->GetWaterBodyComponent();
//
// 		if (CurrWaterBodyComponent != nullptr)
// 		{
// 			// accumulate bounds
// 			FBoxSphereBounds WorldBounds;
// 			CurrWaterBody->GetActorBounds(true, WorldBounds.Origin, WorldBounds.BoxExtent);				
//
// 			CombinedWorldBoundsBuilder += WorldBounds;
// 		}
// 	}
// 	FBoxSphereBounds CombinedBounds(CombinedWorldBoundsBuilder);
//
// 	if (CombinedBounds.BoxExtent.Length() < SMALL_NUMBER)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - river bodies have zero bounds"));
// 		return;
// 	}
// 	
// 	SystemPos = CombinedBounds.Origin - FVector(0, 0, CombinedBounds.BoxExtent.Z);
// 	
// 	////////////// Test 수정 
// 	
// 	
// 	
// 	
// 	
// 	if (bDoesTest)
// 	{
// 		if (TestIndex >= 0 && TestIndex < 4)
// 		{
// 			// 1. 위치 설정 (x, y, z)
// 			FVector CenterLocation(CombinedBounds.Origin);
//
// 			// 2. 박스의 크기 설정 (전체 길이 l, m, k의 절반 값인 Extent 계산)
// 			FVector BoxExtent(CombinedBounds.BoxExtent);
//
// 			// 3. 디버그 박스 그리기
// 			DrawDebugBox(
// 				GetWorld(),                 // 현재 월드 컨텍스트
// 				CenterLocation,             // 중심 위치
// 				BoxExtent,                  // 각 축의 절반 크기 (Extent)
// 				FColor::Green,              // 선 색상 (예: 녹색)
// 				false,                      // 영구 유지 여부 (false면 특정 시간 동안만 표시)
// 				10.0f,                       // 표시할 시간 (초 단위, false일 때 작동)
// 				0,                          // 깊이 우선순위 (기본값 0)
// 				3.0f                        // 선 두께 (Thickness)
// 			);
// 			
// 			int TestIndexX = TestIndex % 2;
// 			int TestIndexY = TestIndex / 2;
// 			
// 			FVector ZeroPoint = CombinedBounds.Origin - FVector(CombinedBounds.BoxExtent.X, CombinedBounds.BoxExtent.Y, 0);
// 			
// 			float UnitX = CombinedBounds.BoxExtent.X/2 * 2;
// 			float UnitY = CombinedBounds.BoxExtent.Y/2 * 2;
// 			
// 			SystemPos = ZeroPoint + FVector(CombinedBounds.BoxExtent.X/2 + UnitX * TestIndexX, 
// 				CombinedBounds.BoxExtent.Y/2 + UnitY * TestIndexY, 0);
// 		
// 			DrawDebugBox(
// 				GetWorld(),                 // 현재 월드 컨텍스트
// 				SystemPos,             // 중심 위치
// 				FVector(BoxExtent.X/2, BoxExtent.Y/2, BoxExtent.Z),                  // 각 축의 절반 크기 (Extent)
// 				FColor::Red,              // 선 색상 (예: 녹색)
// 				false,                      // 영구 유지 여부 (false면 특정 시간 동안만 표시)
// 				10.0f,                       // 표시할 시간 (초 단위, false일 때 작동)
// 				0,                          // 깊이 우선순위 (기본값 0)
// 				3.0f                        // 선 두께 (Thickness)
// 			);
// 			
// 		}
// 	}
// 	////////////// Test 수정 끝
// 	
// 	RiverSimSystem = NewObject<UNiagaraComponent>(this, NAME_None, RF_Public);
// 	RiverSimSystem->bUseAttachParentBound = false;
// 	RiverSimSystem->SetWorldLocation(SystemPos);
//
// 	bool bReadBakedSim = RenderState == EShallowWaterRenderState::BakedSim || RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::WaterComponent;
//
// 	if (GetWorld() && GetWorld()->bIsWorldInitialized)
// 	{
// 		if (!RiverSimSystem->IsRegistered())
// 		{
// 			RiverSimSystem->RegisterComponentWithWorld(GetWorld());
// 		}
//
// 		RiverSimSystem->SetVisibleFlag(true);
// 		RiverSimSystem->SetAsset(NiagaraRiverSimulation);
// 							
// 		// convert to raw ptr array for function library
// 		if (!bReadBakedSim && bUseCapture)
// 		{
// 			// landscape captures
// 			TArray<AActor*> LandscapeBottomContourActorsRawPtr;
// 			LandscapeBottomContourActorsRawPtr.Add(nullptr);
// 			for (TSoftObjectPtr<AActor> CurrLandscapeActor : BottomContourLandscapeActors)
// 			{
// 				// only accept Landscapes and LandscapeStreamingProxies
// 				if (!Cast<ALandscape>(CurrLandscapeActor.Get()) && !Cast<ALandscapeStreamingProxy>(CurrLandscapeActor.Get()))
// 				{
// 					UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - Landscape bottom contour actors can only be ALandscape actors or ALandscapeStreamingProxy actors"));
// 					continue;
// 				}
//
// 				LandscapeBottomContourActorsRawPtr.Add(CurrLandscapeActor.Get());
// 			}
// 			FBoxSphereBounds LandscapeBottomContourBounds = InitializeCaptureDI("User.LandscapeBottomCapture", LandscapeBottomContourActorsRawPtr);
// 		
// 			// undilated captures
// 			TArray<AActor*> BottomContourActorsRawPtr;
// 			BottomContourActorsRawPtr.Add(nullptr);
// 			AddActorsToRawArray(BottomContourActors, BottomContourActorsRawPtr);			
// 			AddTaggedActorsToArray(BottomContourTags, BottomContourActorsRawPtr);
// 			FBoxSphereBounds CombinedBottomContourBounds = InitializeCaptureDI("User.BottomCapture", BottomContourActorsRawPtr);
// 			FBoxSphereBounds CombinedBottomContourBoundsUnder = InitializeCaptureDI("User.BottomCaptureUnder", BottomContourActorsRawPtr);
//
// 			// Dilated capture
// 			TArray<AActor*> DilatedBottomContourActorsRawPtr;
// 			DilatedBottomContourActorsRawPtr.Add(nullptr);
// 			AddActorsToRawArray(DilatedBottomContourActors, DilatedBottomContourActorsRawPtr);
// 			AddTaggedActorsToArray(DilatedBottomContourTags, DilatedBottomContourActorsRawPtr);
//
// 			FBoxSphereBounds DilatedCombinedBottomContourBounds = InitializeCaptureDI("User.DilatedBottomCapture", DilatedBottomContourActorsRawPtr);
// 			FBoxSphereBounds DilatedCombinedBottomContourBoundsUnder = InitializeCaptureDI("User.DilatedBottomCaptureUnder", DilatedBottomContourActorsRawPtr);
//
// 			// reinitialize and set variables on the system
// 			RiverSimSystem->ReinitializeSystem();
//
// 			RiverSimSystem->SetVariableFloat(FName("LandscapeCaptureOffset"), LandscapeBottomContourBounds.Origin.Z + LandscapeBottomContourBounds.BoxExtent.Z + BottomContourCaptureOffset);			
//
// 			RiverSimSystem->SetVariableFloat(FName("CaptureOffset"), CombinedBottomContourBounds.Origin.Z + CombinedBottomContourBounds.BoxExtent.Z + BottomContourCaptureOffset);
// 			RiverSimSystem->SetVariableFloat(FName("DilatedCaptureOffset"), DilatedCombinedBottomContourBounds.Origin.Z + DilatedCombinedBottomContourBounds.BoxExtent.Z + BottomContourCaptureOffset);
// 			
// 			RiverSimSystem->SetVariableFloat(FName("CaptureOffsetUnder"), CombinedBottomContourBounds.Origin.Z - CombinedBottomContourBounds.BoxExtent.Z - BottomContourCaptureOffset);
// 			RiverSimSystem->SetVariableFloat(FName("DilatedCaptureOffsetUnder"), DilatedCombinedBottomContourBounds.Origin.Z - DilatedCombinedBottomContourBounds.BoxExtent.Z - BottomContourCaptureOffset);
// 		}
// 		else
// 		{
// 			RiverSimSystem->ReinitializeSystem();
// 		}
// 	}
// 	else
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - World not initialized"));
// 		return;
// 	}
// 	
//
// 	if (RiverSimSystem == nullptr)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - Cannot spawn river system"));
// 		return;
// 	}
//
// 	// look for the water info texture
// 	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
// 	{
// 		if (AWaterZone* WaterZone = CurrWaterBody->GetWaterBodyComponent()->GetWaterZone())
// 		{			
// 			const TObjectPtr<UTextureRenderTarget2DArray> NewWaterInfoTexture = WaterZone->WaterInfoTextureArray;
// 			if (NewWaterInfoTexture == nullptr)
// 			{
// 				WaterZone->GetOnWaterInfoTextureArrayCreated().RemoveDynamic(this, &UShallowWaterRiverComponent::OnWaterInfoTextureArrayCreated);
// 				WaterZone->GetOnWaterInfoTextureArrayCreated().AddDynamic(this, &UShallowWaterRiverComponent::OnWaterInfoTextureArrayCreated);
// 			}
// 			else
// 			{
// 				OnWaterInfoTextureArrayCreated(NewWaterInfoTexture);
// 			}			
//
// 			const int32 PlayerIndex = 0;
// 			FVector ZoneLocation;
// 			WaterZone->GetDynamicWaterInfoCenter(PlayerIndex, ZoneLocation);
// 			const FVector2D ZoneExtent = FVector2D(WaterZone->GetDynamicWaterInfoExtent());
// 			const FVector2D WaterHeightExtents = FVector2D(WaterZone->GetWaterHeightExtents());
// 			const float GroundZMin = WaterZone->GetGroundZMin();
//
// 			RiverSimSystem->SetVariableVec2(FName("WaterZoneLocation"), FVector2D(ZoneLocation));
// 			RiverSimSystem->SetVariableVec2(FName("WaterZoneExtent"), ZoneExtent);
// 			RiverSimSystem->SetVariableInt(FName("WaterZoneIdx"), WaterZone->GetWaterZoneIndex());
// 			
// 			break;
// 		}
// 	}
//
// 	RiverSimSystem->Activate();
// 	
// 	WorldGridSize = 2.0f * FVector2D(CombinedBounds.BoxExtent.X, CombinedBounds.BoxExtent.Y);
//
// 	if (WorldGridSize.Length() < 1e-8)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Bake() - Simulation grid has (0,0) size."));
// 		return;
// 	}
// 	
// 	/////////////////////// 수정 Test
// 	
// 	if (bDoesTest)
// 	{
// 		if (TestIndex >= 0 && TestIndex < 4)
// 		{
// 			WorldGridSize /= 2;
// 			
// 			
// 		}
// 	}
// 	
// 	
// 	////////////// Test 수정 끝
//
// 	RiverSimSystem->SetVariableVec2(FName("WorldGridSize"), WorldGridSize);
// 	RiverSimSystem->SetVariableInt(FName("ResolutionMaxAxis"), ResolutionMaxAxis);
//
// 	// #todo(dmp): would be better to initialize the user var inside of niagara rather than recomputing res here
// 	SimRes = FVector2D(ResolutionMaxAxis, ResolutionMaxAxis * WorldGridSize.Y / WorldGridSize.X);
// 	if (WorldGridSize.Y > WorldGridSize.X)
// 	{
// 		SimRes = FVector2D(ResolutionMaxAxis * WorldGridSize.X / WorldGridSize.Y, ResolutionMaxAxis);
// 	}
// 	RiverSimSystem->SetVariableVec2(FName("SimRes"), SimRes);
//
// 	// pad out source's box height a so it intersects the sim plane.  This value doesn't matter much so we hardcode it
// 	float Overshoot = 1000.f;
// 	float FinalSourceHeight = 2. * CombinedBounds.BoxExtent.Z + Overshoot;
//
// 	// Get sources	
// 	TArray<FVector> SourcePosArray;
// 	TArray<FVector3f> FullSourceSizeArray;
// 	TArray<float> FullSourceAngleArray;
// 	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : SourceRiverWaterBodies)
// 	{
// 		FVector CurrSourcePos;
// 		float CurrSourceWidth;
// 		float CurrSourceDepth;
// 		FVector CurrSourceDir;
// 		if (!QueryWaterAtSplinePoint(CurrWaterBody, 0, CurrSourcePos, CurrSourceDir, CurrSourceWidth, CurrSourceDepth))
// 		{
// 			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - water source query failed"));
// 			continue;
// 		}		
// 		
// 		FVector FullSourcePos = CurrSourcePos - FVector(0, 0, .5 * FinalSourceHeight) + FVector(CurrSourceDir.X, CurrSourceDir.Y, 0) * .5 * SourceSize;
// 		FVector FullSourceSize = FVector(CurrSourceWidth, SourceSize, FinalSourceHeight); 
// 		
// 		CurrSourceDir = FVector(CurrSourceDir.X, CurrSourceDir.Y, 0);
// 		CurrSourceDir.Normalize();
// 		
// 		FVector BaseVector = {0,1,0};	
// 		double FullSourceAngle =  FMath::Acos(FVector::DotProduct(BaseVector, CurrSourceDir));
// 		
// 		FVector AxisToUse = FVector::CrossProduct(BaseVector, CurrSourceDir);
// 		AxisToUse.Normalize();
//
// #if ENABLE_DRAW_DEBUG
// 		if (bShallowWaterRiverDebugVisualize)
// 		{
// 			FQuat TmpQ = FQuat::MakeFromRotationVector(AxisToUse * FullSourceAngle);		
// 			DrawDebugBox(GetWorld() , (FVector) FullSourcePos, .5 * FullSourceSize, TmpQ, FColor::Green, true);		
// 		}
// #endif
//
// 		// flip axis so we don't need to store the vector itself
// 		if (AxisToUse.Z < 0)
// 		{
// 			FullSourceAngle *= -1;
// 		}
//
//
// 		SourcePosArray.Add(FullSourcePos);
// 		FullSourceSizeArray.Add(FVector3f(FullSourceSize));
// 		FullSourceAngleArray.Add(FullSourceAngle);				
// 	}
// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(RiverSimSystem, "User.SourcePosArray",SourcePosArray);
// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(RiverSimSystem, "User.SourceSizeArray", FullSourceSizeArray);
// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(RiverSimSystem, "User.SourceAngleArray", FullSourceAngleArray);	
//
// 	// get sinks		
// 	TArray<FVector> SinkPosArray;
// 	TArray<FVector3f> FullSinkSizeArray;
// 	TArray<float> FullSinkAngleArray;
//
// 	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : SinkRiverWaterBodies)
// 	{
// 		FVector SinkPos(0, 0, 0);
// 		float SinkWidth = 1;
// 		float SinkDepth = 1;
// 		FVector SinkDir(1, 0, 0);
// 		if (!QueryWaterAtSplinePoint(CurrWaterBody, -1, SinkPos, SinkDir, SinkWidth, SinkDepth))
// 		{
// 			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - water sink query failed"));
// 			continue;
// 		}
//
// 		// height of the sink box doesn't matter
// 		float SinkBoxHeight = 100000;
// 		FVector FullSinkSize = FVector(SinkWidth, SourceSize, SinkBoxHeight);
// 		
// 		SinkDir = FVector(SinkDir.X, SinkDir.Y, 0);
// 		SinkDir.Normalize();
// 		
// 		FVector BaseVector = {0,1,0};	
// 		double FullSinkAngle =  FMath::Acos(FVector::DotProduct(BaseVector, SinkDir));
// 		
// 		FVector AxisToUse = FVector::CrossProduct(BaseVector, SinkDir);
// 		AxisToUse.Normalize();
//
// #if ENABLE_DRAW_DEBUG
// 		if (bShallowWaterRiverDebugVisualize)
// 		{
// 			FQuat TmpQ = FQuat::MakeFromRotationVector(AxisToUse * FullSinkAngle);		
// 			DrawDebugBox(GetWorld() , (FVector) SinkPos, .5 * FullSinkSize, TmpQ, FColor::Red, true);		
// 		}
// #endif
//
// 		// flip axis so we don't need to store the vector itself
// 		if (AxisToUse.Z < 0)
// 		{
// 			FullSinkAngle *= -1;
// 		}
// 		
// 		SinkPosArray.Add(SinkPos);
// 		FullSinkSizeArray.Add(FVector3f(FullSinkSize));
// 		FullSinkAngleArray.Add(FullSinkAngle);				
// 	}
// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(RiverSimSystem, "User.SinkPosArray",SinkPosArray);
// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(RiverSimSystem, "User.SinkSizeArray", FullSinkSizeArray);
// 	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(RiverSimSystem, "User.SinkAngleArray", FullSinkAngleArray);		
//
// 	RiverSimSystem->SetVariableFloat(FName("SimSpeed"), SimSpeed);
// 	RiverSimSystem->SetVariableInt(FName("NumSteps"), NumSteps);
//
// 	RiverSimSystem->SetVariableBool(FName("MatchSpline"), bMatchSpline);
// 	RiverSimSystem->SetVariableFloat(FName("RemoveOutsideSplineAmount"), RemoveOutsideSplineAmount);
// 	RiverSimSystem->SetVariableFloat(FName("SplineHeightMatchingAmount"), MatchSplineHeightAmount);
// 	
// 	BakedWaterSurfaceRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
// 	BakedWaterSurfaceRT->InitAutoFormat(1, 1);
// 	RiverSimSystem->SetVariableTextureRenderTarget(FName("SimGridRT"), BakedWaterSurfaceRT);
// 	
// 	BakedFoamRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
// 	BakedFoamRT->InitAutoFormat(1, 1);
// 	RiverSimSystem->SetVariableTextureRenderTarget(FName("FoamRT"), BakedFoamRT);
// 	
// 	BakedWaterSurfaceNormalRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
// 	BakedWaterSurfaceNormalRT->InitAutoFormat(1, 1);
// 	RiverSimSystem->SetVariableTextureRenderTarget(FName("NormalRT"), BakedWaterSurfaceNormalRT);
//
// 	RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
//
// 	RiverSimSystem->SetVariableFloat(FName("BottomContourCollisionDilation"), BottomContourCollisionDilation);
// 	
// 	RiverSimSystem->SetVariableInt(FName("ExtrapolationHalfWidth"), SmoothingWidth);
// 	RiverSimSystem->SetVariableFloat(FName("SmoothingHeightCutoff"), SmoothingCutoff);
//
// 	if (BakedWaterSurfaceTexture != nullptr && BakedFoamTexture != nullptr && BakedWaterSurfaceNormalTexture != nullptr)
// 	{
// 		RiverSimSystem->SetVariableTexture(FName("BakedSimTexture"), BakedWaterSurfaceTexture);
// 		RiverSimSystem->SetVariableTexture(FName("BakedFoamTexture"), BakedFoamTexture);
// 		RiverSimSystem->SetVariableTexture(FName("BakedWaterSurfaceNormalTexture"), BakedWaterSurfaceNormalTexture);
// 	}
//
// 	TObjectPtr<UTextureRenderTarget2D>  OceanPatchNormalRT = GetSharedFFTOceanPatchNormalRTFromSubsystem(GetWorld());
//
// 	if (OceanPatchNormalRT == nullptr)
// 	{
// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - ocean patch normal RT is not initialized"));
// 		return;
// 	}
//
// 	NormalDetailRT = OceanPatchNormalRT;
// 	RiverSimSystem->SetVariableTextureRenderTarget(FName("NormalDetailRT"), NormalDetailRT);
//
// #if WITH_EDITOR
// 	// run the live sim and make sure that we enable/disable cpu throttling	
// 	if (!bReadBakedSim)
// 	{		
// 		GEditor->ShouldDisableCPUThrottlingDelegates.Add(UEditorEngine::FShouldDisableCPUThrottling::CreateUObject(this, &UShallowWaterRiverComponent::ShouldDisableCPUThrottling));
// 		ShouldDisableCPUThrottlingDelegateHandle = GEditor->ShouldDisableCPUThrottlingDelegates.Last().GetHandle();
// 	}
// 	else
// 	{
// 		GEditor->ShouldDisableCPUThrottlingDelegates.RemoveAll([this](const UEditorEngine::FShouldDisableCPUThrottling& Delegate)
// 		{
// 			return Delegate.GetHandle() == ShouldDisableCPUThrottlingDelegateHandle;
// 		});
// 	}
// #endif
//
// 	bIsInitialized = true;
// }

void UShallowWaterRiverComponent::Rebuild()
{	
	bIsInitialized = false;
	bTickInitialize = false;

	if (NiagaraRiverSimulation == nullptr)
	{
		NiagaraRiverSimulation = LoadObject<UNiagaraSystem>(nullptr, TEXT("/WaterAdvanced/Niagara/Systems/Grid2D_SW_River.Grid2D_SW_River"));
	}

	if (RiverSimSystem != nullptr)
	{
		RiverSimSystem->SetActive(false);
		RiverSimSystem->DestroyComponent();
		RiverSimSystem = nullptr;
	}
	
	if (ResolutionMaxAxis <= 0)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - resolution must be greater than 0"));
		return;
	}

	if (NumSteps <= 0)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - num steps must be greater than 0"));
		return;
	}

	if (SimSpeed <= 1e-8)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - speed must be greater than zero"));
		return;
	}

	if (NiagaraRiverSimulation == nullptr)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - null Niagara system asset"));
		return;
	}

	AllWaterBodies.Empty();

	// collect all the water bodies	
	if (SourceRiverWaterBodies.Num() != 0)
	{
		for (TSoftObjectPtr<AWaterBody > CurrWaterBody : SourceRiverWaterBodies)
		{
			if (CurrWaterBody)
			{
				AllWaterBodies.Add(CurrWaterBody);
			}
			else 
			{
				UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - skipping null water body actor found"));
				continue;
			}
		}
	}
	else	
	{
		UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - No source water bodies specified"));
		return;
	}
	
	if (AllWaterBodies.Num() == 0)
	{
		UE_LOG(LogShallowWater, Verbose, TEXT("UShallowWaterRiverComponent::Rebuild() - No valid source water bodies specified"));
		return;
	}

	bool HasValidSinks = false;
	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : SinkRiverWaterBodies)
	{
		if (CurrWaterBody != nullptr)
		{
			HasValidSinks = true;
			AllWaterBodies.Add(CurrWaterBody);			
		}
		else
		{
			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - skipping null sink water body actor found"));
			continue;
		}
	}
	
	// Source도 Sink도 아닌 애들 넣기. 
	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : LinkWaterBodies)
	{
		if (CurrWaterBody != nullptr)
		{
			AllWaterBodies.Add(CurrWaterBody);
		}
	}

	// flush all debug draw lines
#if ENABLE_DRAW_DEBUG
	FlushPersistentDebugLines(GetWorld());
#endif

	if (!HasValidSinks)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - no valid sinks, using the first source as a sink"));
		SinkRiverWaterBodies.Add(*AllWaterBodies.CreateConstIterator());
	}

	// accumulate bounding box for river water bodies
	FBoxSphereBounds::Builder CombinedWorldBoundsBuilder;
	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
	{
		TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = CurrWaterBody->GetWaterBodyComponent();

		if (CurrWaterBodyComponent != nullptr)
		{
			// accumulate bounds
			FBoxSphereBounds WorldBounds;
			CurrWaterBody->GetActorBounds(true, WorldBounds.Origin, WorldBounds.BoxExtent);				

			CombinedWorldBoundsBuilder += WorldBounds;
		}
	}
	FBoxSphereBounds CombinedBounds(CombinedWorldBoundsBuilder);

	if (CombinedBounds.BoxExtent.Length() < SMALL_NUMBER)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - river bodies have zero bounds"));
		return;
	}
	
	SystemPos = CombinedBounds.Origin - FVector(0, 0, CombinedBounds.BoxExtent.Z);
	
	////////////// Test 수정 
	
	int32 GridSizeX = FMath::Max(1, ChunkGridDimensions.X);
	int32 GridSizeY = FMath::Max(1, ChunkGridDimensions.Y);
	GridSizeX = FMath::Min(GridSizeX, 1000);
	GridSizeY = FMath::Min(GridSizeY, 1000);
	
	// 1. 임시 목표 월드 크기
	FVector2D RawWorldSize = 2.0f * FVector2D(CombinedBounds.BoxExtent.X, CombinedBounds.BoxExtent.Y);
	FVector2D TempChunkSize = RawWorldSize / FVector2D(GridSizeX, GridSizeY);
	
	// 2. 1픽셀당 월드 크기(ExpectedSimDx)를 결정
	SimDx = FMath::Max(TempChunkSize.X, TempChunkSize.Y) / ResolutionMaxAxis;
	
	// 3. 목표 크기를 픽셀 단위(정수)로 변환 (여기서 소수점 오차를 한 번 끊어냄)
	BaseChunkRes.X = FMath::RoundToInt(TempChunkSize.X / SimDx);
	BaseChunkRes.Y = FMath::RoundToInt(TempChunkSize.Y / SimDx);
	ChunkRes = FVector2D(BaseChunkRes.X + 2*MarginCells, BaseChunkRes.Y + 2*MarginCells); 
	
	SimRes.X = BaseChunkRes.X * GridSizeX;
	SimRes.Y = BaseChunkRes.Y * GridSizeY;
	
	// 4. 결정된 정수 픽셀을 바탕으로 "완벽하게 스냅된 월드 크기"를 역산
	BaseChunkSize.X = BaseChunkRes.X * SimDx;
	BaseChunkSize.Y = BaseChunkRes.Y * SimDx;
	ChunkSize = BaseChunkSize + FVector2D(AlignedOverlapMargin * 2.0f, AlignedOverlapMargin * 2.0f);
	
	WorldGridSize.X = BaseChunkSize.X * GridSizeX;
	WorldGridSize.Y = BaseChunkSize.Y * GridSizeY;
	
	// 5. 오버랩 마진 계산
	AlignedOverlapMargin = SimDx * MarginCells;
	
	
	// 6. 스냅된 크기를 바탕으로 전체 기준점(SystemPos)과 좌측 하단 시작점 재계산
	//FVector BottomLeftOrigin = CombinedBounds.Origin - FVector(CombinedBounds.BoxExtent.X, CombinedBounds.BoxExtent.Y, CombinedBounds.BoxExtent.Z);
	FVector BottomLeftOrigin = CombinedBounds.Origin - FVector(CombinedBounds.BoxExtent.X, CombinedBounds.BoxExtent.Y, 0.f);
	//SystemPos = BottomLeftOrigin + FVector(WorldGridSize.X * 0.5f, WorldGridSize.Y * 0.5f, CombinedBounds.BoxExtent.Z); // 전체 정중앙
	SystemPos = BottomLeftOrigin + FVector(WorldGridSize.X * 0.5f, WorldGridSize.Y * 0.5f, 0.f); // 전체 정중앙
	
	// 기존 청크 초기화
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		if (Chunk.RiverSimSystem)
		{
			Chunk.RiverSimSystem->SetActive(false);
			Chunk.RiverSimSystem->DestroyComponent();
		}
	}
	ShallowWaterChunks.Empty();
	
	bool bReadBakedSim = RenderState == EShallowWaterRenderState::BakedSim || RenderState ==
		EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::WaterComponent;

	//7. 청크 생성 루프
	for (int32 Y = 0; Y < GridSizeY; ++Y)
	{
		for (int32 X = 0; X < GridSizeX; ++X)
		{
			FShallowWaterChunk NewChunk;
			NewChunk.GridIndex = FIntPoint(X, Y);
			// 🚨 핵심: 실제 시뮬레이션 영역(ChunkWorldSize)은 마진을 더해 확장함
			NewChunk.ChunkWorldSize = ChunkSize;

			// 청크의 중심 좌표 계산
			FVector ChunkCenterOffset = FVector(
				(X * BaseChunkSize.X) + (BaseChunkSize.X * 0.5f),
				(Y * BaseChunkSize.Y) + (BaseChunkSize.Y * 0.5f),
				0.0f
			);
			NewChunk.SystemPos = BottomLeftOrigin + ChunkCenterOffset;

			// 나이아가라 시스템 스폰
			NewChunk.RiverSimSystem = NewObject<UNiagaraComponent>(this, NAME_None, RF_Public);
			NewChunk.RiverSimSystem->SetAutoActivate(false); //
			NewChunk.RiverSimSystem->bUseAttachParentBound = false;
			NewChunk.RiverSimSystem->SetWorldLocation(NewChunk.SystemPos);
			
			
			if (GetWorld() && GetWorld()->bIsWorldInitialized)
			{
				if (!NewChunk.RiverSimSystem->IsRegistered())
				{
					NewChunk.RiverSimSystem->RegisterComponentWithWorld(GetWorld());
				}

				NewChunk.RiverSimSystem->SetVisibleFlag(true);
				NewChunk.RiverSimSystem->SetAsset(NiagaraRiverSimulation);
			}
			
			int32 ChunkIdx = ShallowWaterChunks.Add(NewChunk);
		}
	}
	
	// 기존엔 이거 하나로 시뮬레이션 했지만 이젠 각 청크의 나이아가라 시스템이 시뮬레이션을 맡음.
	// RiverSimSystem은 베이크 후 통합된 거품 스폰하기.
	RiverSimSystem = NewObject<UNiagaraComponent>(this, NAME_None, RF_Public);
	//RiverSimSystem->SetAutoActivate(false);
	RiverSimSystem->bUseAttachParentBound = false;
	RiverSimSystem->SetWorldLocation(SystemPos); // 전체 바운딩 박스의 기준점
	
	if (GetWorld() && GetWorld()->bIsWorldInitialized)
	{
		if (!RiverSimSystem->IsRegistered())
		{
			RiverSimSystem->RegisterComponentWithWorld(GetWorld());
		}

		RiverSimSystem->SetVisibleFlag(true);
		RiverSimSystem->SetAsset(NiagaraRiverSimulation);
	}
	

	// 파라미터는 청크 크기가 아닌 '전체 WorldGridSize'와 '합쳐진 SimRes'를 넣습니다.
	RiverSimSystem->SetVariableVec2(FName("WorldGridSize"), WorldGridSize);
	RiverSimSystem->SetVariableInt(FName("ResolutionMaxAxis"), ResolutionMaxAxis);
	RiverSimSystem->SetVariableVec2(FName("SimRes"), SimRes);
	
	// 딱히 쓸때없지만 일단 RT 세팅해둘까.
	BakedWaterSurfaceRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
	BakedWaterSurfaceRT->InitAutoFormat(1, 1);
	RiverSimSystem->SetVariableTextureRenderTarget(FName("SimGridRT"), BakedWaterSurfaceRT);
	
	BakedFoamRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
	BakedFoamRT->InitAutoFormat(1, 1);
	RiverSimSystem->SetVariableTextureRenderTarget(FName("FoamRT"), BakedFoamRT);
	
	BakedWaterSurfaceNormalRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
	BakedWaterSurfaceNormalRT->InitAutoFormat(1, 1);
	RiverSimSystem->SetVariableTextureRenderTarget(FName("NormalRT"), BakedWaterSurfaceNormalRT);
	
	// if (BakedWaterSurfaceTexture != nullptr && BakedFoamTexture != nullptr && BakedWaterSurfaceNormalTexture != nullptr)
	// {
	// 	RiverSimSystem->SetVariableTexture(FName("BakedSimTexture"), BakedWaterSurfaceTexture);
	// 	RiverSimSystem->SetVariableTexture(FName("BakedFoamTexture"), BakedFoamTexture);
	// 	RiverSimSystem->SetVariableTexture(FName("BakedWaterSurfaceNormalTexture"), BakedWaterSurfaceNormalTexture);
	// }
	
	// 캡처 생성
	// [TODO] 씬 캡처 (BottomContour) 로직은 각 청크의 바운딩 박스에 걸치는 액터만 
	// 필터링해서 InitializeCaptureDI를 호출하도록 분리해야 함.
	if (GetWorld() && GetWorld()->bIsWorldInitialized)
	{
		if (!bReadBakedSim && bUseCapture)
		{
			// landscape captures
			TArray<AActor*> LandscapeBottomContourActorsRawPtr;
			LandscapeBottomContourActorsRawPtr.Add(nullptr);
			for (TSoftObjectPtr<AActor> CurrLandscapeActor : BottomContourLandscapeActors)
			{
				// only accept Landscapes and LandscapeStreamingProxies
				if (!Cast<ALandscape>(CurrLandscapeActor.Get()) && !Cast<
					ALandscapeStreamingProxy>(CurrLandscapeActor.Get()))
				{
					UE_LOG(LogShallowWater, Warning,
						   TEXT(
							   "UShallowWaterRiverComponent::Rebuild() - Landscape bottom contour actors can only be ALandscape actors or ALandscapeStreamingProxy actors"
						   ));
					continue;
				}

				LandscapeBottomContourActorsRawPtr.Add(CurrLandscapeActor.Get());
			}
			
			// undilated captures
			TArray<AActor*> BottomContourActorsRawPtr;
			BottomContourActorsRawPtr.Add(nullptr);
			AddActorsToRawArray(BottomContourActors, BottomContourActorsRawPtr);
			AddTaggedActorsToArray(BottomContourTags, BottomContourActorsRawPtr);
			
			// Dilated capture
			TArray<AActor*> DilatedBottomContourActorsRawPtr;
			DilatedBottomContourActorsRawPtr.Add(nullptr);
			AddActorsToRawArray(DilatedBottomContourActors, DilatedBottomContourActorsRawPtr);
			AddTaggedActorsToArray(DilatedBottomContourTags, DilatedBottomContourActorsRawPtr);

			float ChunkOrthoWidth = FMath::Max(ChunkSize.X, ChunkSize.Y);
			//float ChunkOrthoWidth = ChunkSize.Y;
			int32 MaxRes = FMath::Max(ChunkRes.X, ChunkRes.Y);
			//FIntPoint ChunkOrthoRes = FIntPoint(ChunkRes.X, ChunkRes.Y);
			FIntPoint ChunkOrthoRes = FIntPoint(MaxRes, MaxRes);
			
			for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
			{
				FBoxSphereBounds LandscapeBottomContourBounds = InitializeCaptureDI(
					Chunk.RiverSimSystem, "User.LandscapeBottomCapture", LandscapeBottomContourActorsRawPtr, ChunkOrthoRes, ChunkOrthoWidth);
				
				// undilated captures
				FBoxSphereBounds CombinedBottomContourBounds = InitializeCaptureDI(
					Chunk.RiverSimSystem, "User.BottomCapture", BottomContourActorsRawPtr, ChunkOrthoRes, ChunkOrthoWidth );
				FBoxSphereBounds CombinedBottomContourBoundsUnder = InitializeCaptureDI(
					Chunk.RiverSimSystem, "User.BottomCaptureUnder", BottomContourActorsRawPtr, ChunkOrthoRes, ChunkOrthoWidth);

				// Dilated capture
				FBoxSphereBounds DilatedCombinedBottomContourBounds = InitializeCaptureDI(
					Chunk.RiverSimSystem,
					"User.DilatedBottomCapture", DilatedBottomContourActorsRawPtr, ChunkOrthoRes, ChunkOrthoWidth);
				FBoxSphereBounds DilatedCombinedBottomContourBoundsUnder = InitializeCaptureDI(
					Chunk.RiverSimSystem,
					"User.DilatedBottomCaptureUnder", DilatedBottomContourActorsRawPtr, ChunkOrthoRes, ChunkOrthoWidth);

				// reinitialize and set variables on the system
				//WaterChunk.RiverSimSystem->ReinitializeSystem();
				Chunk.RiverSimSystem->DestroyInstanceNotComponent();//

				Chunk.RiverSimSystem->SetVariableFloat(FName("LandscapeCaptureOffset"),
															LandscapeBottomContourBounds.Origin.Z +
															LandscapeBottomContourBounds.BoxExtent.Z +
															BottomContourCaptureOffset);

				Chunk.RiverSimSystem->SetVariableFloat(FName("CaptureOffset"),
															CombinedBottomContourBounds.Origin.Z +
															CombinedBottomContourBounds.BoxExtent.Z +
															BottomContourCaptureOffset);
				Chunk.RiverSimSystem->SetVariableFloat(FName("DilatedCaptureOffset"),
															DilatedCombinedBottomContourBounds.Origin.Z +
															DilatedCombinedBottomContourBounds.BoxExtent.Z +
															BottomContourCaptureOffset);

				Chunk.RiverSimSystem->SetVariableFloat(FName("CaptureOffsetUnder"),
															CombinedBottomContourBounds.Origin.Z -
															CombinedBottomContourBounds.BoxExtent.Z -
															BottomContourCaptureOffset);
				Chunk.RiverSimSystem->SetVariableFloat(FName("DilatedCaptureOffsetUnder"),
															DilatedCombinedBottomContourBounds.Origin.Z -
															DilatedCombinedBottomContourBounds.BoxExtent.Z -
															BottomContourCaptureOffset);
			}
		}
		else
		{
			for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
			{
				//Chunk.RiverSimSystem->ReinitializeSystem();
				Chunk.RiverSimSystem->DestroyInstanceNotComponent();
			}
		}
	}
	else
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - World not initialized"));
		return;
	}

	// 워터존에서 Water Info Texture 찾기.
	// look for the water info texture
	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : AllWaterBodies)
	{
		if (AWaterZone* WaterZone = CurrWaterBody->GetWaterBodyComponent()->GetWaterZone())
		{
			const TObjectPtr<UTextureRenderTarget2DArray> NewWaterInfoTexture = WaterZone->WaterInfoTextureArray;
			if (NewWaterInfoTexture == nullptr)
			{
				WaterZone->GetOnWaterInfoTextureArrayCreated().RemoveDynamic(
					this, &UShallowWaterRiverComponent::OnWaterInfoTextureArrayCreated);
				WaterZone->GetOnWaterInfoTextureArrayCreated().AddDynamic(
					this, &UShallowWaterRiverComponent::OnWaterInfoTextureArrayCreated);
			}
			else
			{
				OnWaterInfoTextureArrayCreated(NewWaterInfoTexture);
			}

			const int32 PlayerIndex = 0;
			FVector ZoneLocation;
			WaterZone->GetDynamicWaterInfoCenter(PlayerIndex, ZoneLocation);
			const FVector2D ZoneExtent = FVector2D(WaterZone->GetDynamicWaterInfoExtent());
			const FVector2D WaterHeightExtents = FVector2D(WaterZone->GetWaterHeightExtents());
			const float GroundZMin = WaterZone->GetGroundZMin();

			for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
			{
				Chunk.RiverSimSystem->SetVariableVec2(FName("WaterZoneLocation"), FVector2D(ZoneLocation));
				Chunk.RiverSimSystem->SetVariableVec2(FName("WaterZoneExtent"), ZoneExtent);
				Chunk.RiverSimSystem->SetVariableInt(FName("WaterZoneIdx"), WaterZone->GetWaterZoneIndex());
			}
			
			break;
		}
	}


	// pad out source's box height a so it intersects the sim plane.  This value doesn't matter much so we hardcode it
	float Overshoot = 1000.f;
	float FinalSourceHeight = 2. * CombinedBounds.BoxExtent.Z + Overshoot;

	// Get sources	
	// TArray<FVector> SourcePosArray;
	// TArray<FVector3f> FullSourceSizeArray;
	// TArray<float> FullSourceAngleArray;
	SourcePosArray.Empty();
	SourceSizeArray.Empty();
	SourceAngleArray.Empty();
	
	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : SourceRiverWaterBodies)
	{
		FVector CurrSourcePos;
		float CurrSourceWidth;
		float CurrSourceDepth;
		FVector CurrSourceDir;
		if (!QueryWaterAtSplinePoint(CurrWaterBody, 0, CurrSourcePos, CurrSourceDir, CurrSourceWidth, CurrSourceDepth))
		{
			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - water source query failed"));
			continue;
		}		
		
		FVector FullSourcePos = CurrSourcePos - FVector(0, 0, .5 * FinalSourceHeight) + FVector(CurrSourceDir.X, CurrSourceDir.Y, 0) * .5 * SourceSize;
		FVector FullSourceSize = FVector(CurrSourceWidth, SourceSize, FinalSourceHeight); 
		
		CurrSourceDir = FVector(CurrSourceDir.X, CurrSourceDir.Y, 0);
		CurrSourceDir.Normalize();
		
		FVector BaseVector = {0,1,0};	
		double FullSourceAngle =  FMath::Acos(FVector::DotProduct(BaseVector, CurrSourceDir));
		
		FVector AxisToUse = FVector::CrossProduct(BaseVector, CurrSourceDir);
		AxisToUse.Normalize();

#if ENABLE_DRAW_DEBUG
		if (bShallowWaterRiverDebugVisualize)
		{
			FQuat TmpQ = FQuat::MakeFromRotationVector(AxisToUse * FullSourceAngle);		
			DrawDebugBox(GetWorld() , (FVector) FullSourcePos, .5 * FullSourceSize, TmpQ, FColor::Green, true);		
		}
#endif

		// flip axis so we don't need to store the vector itself
		if (AxisToUse.Z < 0)
		{
			FullSourceAngle *= -1;
		}


		SourcePosArray.Add(FullSourcePos);
		SourceSizeArray.Add(FVector3f(FullSourceSize));
		SourceAngleArray.Add(FullSourceAngle);				
	}
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(Chunk.RiverSimSystem, "User.SourcePosArray",SourcePosArray);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(Chunk.RiverSimSystem, "User.SourceSizeArray", SourceSizeArray);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(Chunk.RiverSimSystem, "User.SourceAngleArray", SourceAngleArray);
	}

	
	// get sinks		
	TArray<FVector> SinkPosArray;
	TArray<FVector3f> FullSinkSizeArray;
	TArray<float> FullSinkAngleArray;

	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : SinkRiverWaterBodies)
	{
		FVector SinkPos(0, 0, 0);
		float SinkWidth = 1;
		float SinkDepth = 1;
		FVector SinkDir(1, 0, 0);
		if (!QueryWaterAtSplinePoint(CurrWaterBody, -1, SinkPos, SinkDir, SinkWidth, SinkDepth))
		{
			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - water sink query failed"));
			continue;
		}

		// height of the sink box doesn't matter
		float SinkBoxHeight = 100000;
		FVector FullSinkSize = FVector(SinkWidth, SourceSize, SinkBoxHeight);
		
		SinkDir = FVector(SinkDir.X, SinkDir.Y, 0);
		SinkDir.Normalize();
		
		FVector BaseVector = {0,1,0};	
		double FullSinkAngle =  FMath::Acos(FVector::DotProduct(BaseVector, SinkDir));
		
		FVector AxisToUse = FVector::CrossProduct(BaseVector, SinkDir);
		AxisToUse.Normalize();

#if ENABLE_DRAW_DEBUG
		if (bShallowWaterRiverDebugVisualize)
		{
			FQuat TmpQ = FQuat::MakeFromRotationVector(AxisToUse * FullSinkAngle);		
			DrawDebugBox(GetWorld() , (FVector) SinkPos, .5 * FullSinkSize, TmpQ, FColor::Red, true);		
		}
#endif

		// flip axis so we don't need to store the vector itself
		if (AxisToUse.Z < 0)
		{
			FullSinkAngle *= -1;
		}
		
		SinkPosArray.Add(SinkPos);
		FullSinkSizeArray.Add(FVector3f(FullSinkSize));
		FullSinkAngleArray.Add(FullSinkAngle);				
	}
	
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(Chunk.RiverSimSystem, "User.SinkPosArray",SinkPosArray);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(Chunk.RiverSimSystem, "User.SinkSizeArray", FullSinkSizeArray);
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(Chunk.RiverSimSystem, "User.SinkAngleArray", FullSinkAngleArray);
	}
	
	///// 렌더타깃등 각종 파라미터 세팅
	
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		Chunk.SimGridRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
		Chunk.SimGridRT->RenderTargetFormat = RTF_RGBA16f; // 16비트 Float 포맷 지정 (음수와 소수점 보존)
		Chunk.SimGridRT->ClearColor = FLinearColor(0, 0, 0, 0);
		Chunk.SimGridRT->InitAutoFormat(1, 1);
		Chunk.RiverSimSystem->SetVariableTextureRenderTarget(FName("SimGridRT"), Chunk.SimGridRT);
		
		Chunk.FoamRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
		Chunk.FoamRT->RenderTargetFormat = RTF_RGBA16f;
		Chunk.FoamRT->InitAutoFormat(1, 1);
		Chunk.RiverSimSystem->SetVariableTextureRenderTarget(FName("FoamRT"), Chunk.FoamRT);
	
		Chunk.NormalRT = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
		Chunk.NormalRT->RenderTargetFormat = RTF_RGBA16f;
		Chunk.NormalRT->InitAutoFormat(1, 1);
		Chunk.RiverSimSystem->SetVariableTextureRenderTarget(FName("NormalRT"), Chunk.NormalRT);

	}
	
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		int32 X = Chunk.GridIndex.X;
		int32 Y = Chunk.GridIndex.Y;

		// 이웃 청크 포인터 초기화
		FShallowWaterChunk* NeighborTop    = (Y + 1 < GridSizeY) ? &ShallowWaterChunks[(Y + 1) * GridSizeX + X] : nullptr;
		FShallowWaterChunk* NeighborBottom = (Y - 1 >= 0)        ? &ShallowWaterChunks[(Y - 1) * GridSizeX + X] : nullptr;
		FShallowWaterChunk* NeighborLeft   = (X - 1 >= 0)        ? &ShallowWaterChunks[Y * GridSizeX + (X - 1)] : nullptr;
		FShallowWaterChunk* NeighborRight  = (X + 1 < GridSizeX) ? &ShallowWaterChunks[Y * GridSizeX + (X + 1)] : nullptr;
		
		
		// 나이아가라 기본 공간 변수 전달
		Chunk.RiverSimSystem->SetVariableVec2(FName("WorldGridSize"), Chunk.ChunkWorldSize);
		Chunk.RiverSimSystem->SetVariableInt(FName("ResolutionMaxAxis"), FMath::Max(ChunkRes.X, ChunkRes.Y));
		Chunk.RiverSimSystem->SetVariableVec2(FName("BaseGridSize"), BaseChunkSize); // 마진 없는 원래 크기
		Chunk.RiverSimSystem->SetVariableInt(FName("MarginCells"), MarginCells);
		Chunk.RiverSimSystem->SetVariableFloat(FName("OverlapMargin"), AlignedOverlapMargin);
		Chunk.RiverSimSystem->SetVariableVec2(FName("GridIndex"), FVector2D(X, Y));
		
		Chunk.RiverSimSystem->Activate();//
		
		// 해상도는 기존 로직(비율 계산)을 청크 크기에 맞춰 재적용
		FVector2D LocalSimRes = FVector2D(BaseChunkRes.X + (MarginCells * 2), BaseChunkRes.Y + (MarginCells * 2));
		Chunk.RiverSimSystem->SetVariableVec2(FName("SimRes"), LocalSimRes);

		Chunk.RiverSimSystem->SetVariableFloat(FName("SimSpeed"), SimSpeed);
		Chunk.RiverSimSystem->SetVariableInt(FName("NumSteps"), NumSteps);

		Chunk.RiverSimSystem->SetVariableBool(FName("MatchSpline"), bMatchSpline);
		Chunk.RiverSimSystem->SetVariableFloat(FName("RemoveOutsideSplineAmount"), RemoveOutsideSplineAmount);
		Chunk.RiverSimSystem->SetVariableFloat(FName("SplineHeightMatchingAmount"), MatchSplineHeightAmount);
	
		
		Chunk.RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
		//Chunk.RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), true);
		// // 거품 생성 여부.
		// Chunk.RiverSimSystem->SetVariableBool(FName("GenerateFoam"), false);
		
		
		// 🚨 [이웃 데이터 공유] 상하좌우 독립적으로 검증 및 바인딩
		// 위(Top) 이웃 처리
		bool bHasTop = (NeighborTop != nullptr && NeighborTop->SimGridRT != nullptr);
		Chunk.RiverSimSystem->SetVariableBool(FName("HasNeighborTop"), bHasTop);
		if (bHasTop)
		{
			Chunk.RiverSimSystem->SetVariableTexture(FName("NeighborRTTop"), NeighborTop->SimGridRT);
			Chunk.RiverSimSystem->SetVariableVec3(FName("NeighborPosTop"), NeighborTop->SystemPos);
		}

		// 아래(Bottom) 이웃 처리
		bool bHasBottom = (NeighborBottom != nullptr && NeighborBottom->SimGridRT != nullptr);
		Chunk.RiverSimSystem->SetVariableBool(FName("HasNeighborBottom"), bHasBottom);
		if (bHasBottom)
		{
			Chunk.RiverSimSystem->SetVariableTexture(FName("NeighborRTBottom"), NeighborBottom->SimGridRT);
			Chunk.RiverSimSystem->SetVariableVec3(FName("NeighborPosBottom"), NeighborBottom->SystemPos);
		}

		// 왼쪽(Left) 이웃 처리
		bool bHasLeft = (NeighborLeft != nullptr && NeighborLeft->SimGridRT != nullptr);
		Chunk.RiverSimSystem->SetVariableBool(FName("HasNeighborLeft"), bHasLeft);
		if (bHasLeft)
		{
			Chunk.RiverSimSystem->SetVariableTexture(FName("NeighborRTLeft"), NeighborLeft->SimGridRT);
			Chunk.RiverSimSystem->SetVariableVec3(FName("NeighborPosLeft"), NeighborLeft->SystemPos);
		}

		// 오른쪽(Right) 이웃 처리
		bool bHasRight = (NeighborRight != nullptr && NeighborRight->SimGridRT != nullptr);
		Chunk.RiverSimSystem->SetVariableBool(FName("HasNeighborRight"), bHasRight);
		if (bHasRight)
		{
			Chunk.RiverSimSystem->SetVariableTexture(FName("NeighborRTRight"), NeighborRight->SimGridRT);
			Chunk.RiverSimSystem->SetVariableVec3(FName("NeighborPosRight"), NeighborRight->SystemPos);
		}
		

		// 앤 뭐지?
		Chunk.RiverSimSystem->SetVariableFloat(FName("BottomContourCollisionDilation"), BottomContourCollisionDilation);
	
		Chunk.RiverSimSystem->SetVariableInt(FName("ExtrapolationHalfWidth"), SmoothingWidth);
		Chunk.RiverSimSystem->SetVariableFloat(FName("SmoothingHeightCutoff"), SmoothingCutoff);
		
		// Bake한 텍스처가 있을 때. 
		if (Chunk.BakedWaterSurfaceTexture != nullptr && Chunk.BakedFoamTexture != nullptr && Chunk.BakedWaterSurfaceNormalTexture != nullptr)
		{
			Chunk.RiverSimSystem->SetVariableTexture(FName("BakedSimTexture"), Chunk.BakedWaterSurfaceTexture);
			Chunk.RiverSimSystem->SetVariableTexture(FName("BakedFoamTexture"), Chunk.BakedFoamTexture);
			Chunk.RiverSimSystem->SetVariableTexture(FName("BakedWaterSurfaceNormalTexture"), Chunk.BakedWaterSurfaceNormalTexture);
		}
				
		// 이건 또 뭘까. 바다 쪽이랑 관련있는거 같은데.
		TObjectPtr<UTextureRenderTarget2D>  OceanPatchNormalRT = GetSharedFFTOceanPatchNormalRTFromSubsystem(GetWorld());

		if (OceanPatchNormalRT == nullptr)
		{
			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - ocean patch normal RT is not initialized"));
			return;
		}

		Chunk.NormalDetailRT = OceanPatchNormalRT;
		Chunk.RiverSimSystem->SetVariableTextureRenderTarget(FName("NormalDetailRT"), NormalDetailRT);
	}

#if WITH_EDITOR
	// run the live sim and make sure that we enable/disable cpu throttling	
	if (!bReadBakedSim)
	{		
		GEditor->ShouldDisableCPUThrottlingDelegates.Add(UEditorEngine::FShouldDisableCPUThrottling::CreateUObject(this, &UShallowWaterRiverComponent::ShouldDisableCPUThrottling));
		ShouldDisableCPUThrottlingDelegateHandle = GEditor->ShouldDisableCPUThrottlingDelegates.Last().GetHandle();
	}
	else
	{
		GEditor->ShouldDisableCPUThrottlingDelegates.RemoveAll([this](const UEditorEngine::FShouldDisableCPUThrottling& Delegate)
		{
			return Delegate.GetHandle() == ShouldDisableCPUThrottlingDelegateHandle;
		});
	}
#endif

	bIsInitialized = true;
	
	// 🚨 모든 세팅이 끝난 직후, 순차 베이킹 시스템을 초기화합니다.
	if (RenderState == EShallowWaterRenderState::LiveSim)
	{
		InitSequentialSimulation(true);
	}
	
	/// 테스트용
	if (ShallowWaterChunks.Num() > 0)
	{
		TestVisibleRT = ShallowWaterChunks[0].SimGridRT;
	}
	
	// 테스트 메시 스폰 및 삭제 
	for (UMeshComponent* TestMesh : TestDebugMeshes)
	{
		TestMesh->DestroyComponent();
	}
	TestDebugMeshes.Empty();
	
	for (TSoftObjectPtr<AWaterBody> CurrWaterBody : AllWaterBodies)
	{
		if (AWaterBody* WaterBody = CurrWaterBody.Get())
		{
			if (UWaterSplineComponent* CurrSpline = WaterBody->GetWaterSpline())
			{
				//FInterpCurveVector& SplinePoints = CurrSpline->GetSplinePointsPosition();
				int PointNum = CurrSpline->GetNumberOfSplinePoints();
				
				// for (FInterpCurvePoint Point : SplinePoints.Points)
				// {
				// 	UStaticMeshComponent* TestMesh = NewObject<UStaticMeshComponent>(this);
				// 	TestMesh->SetStaticMesh(TestDebugMesh);
				// 	TestMesh->SetupAttachment(this); // 현재 워터 컴포넌트에 부착
				// 	TestMesh->RegisterComponent();
				// 	
				// 	TestMesh->SetWorldLocation(Point.OutVal);
				// 	TestDebugMeshes.Add(TestMesh);
				// }
				
				for (int i= 0; i < PointNum; i++)
				{
					UStaticMeshComponent* TestMesh = NewObject<UStaticMeshComponent>(this);
					TestMesh->SetStaticMesh(TestDebugMesh);
					TestMesh->SetupAttachment(this); // 현재 워터 컴포넌트에 부착
					TestMesh->RegisterComponent();
						
					TestMesh->SetWorldLocation(CurrSpline->GetSplinePointAt(i, ESplineCoordinateSpace::World).Position);
					TestDebugMeshes.Add(TestMesh);
					
					
				}
			}
		}
	}
	
	
	
	////////////// Test 수정 끝
}

void UShallowWaterRiverComponent::AddActorsToRawArray(const TArray<TSoftObjectPtr<AActor>> &ActorsArray, TArray<AActor*>& BottomContourActorsRawPtr)
{
	for (TSoftObjectPtr<AActor> CurrActor : ActorsArray)
	{
		AActor* CurrActorRawPtr = CurrActor.Get();

		// if we have a level instance, break it up and add each actor
		if (ALevelInstance* LevelInstancePtr = Cast<ALevelInstance>(CurrActorRawPtr))
		{
			const ULevelInstanceSubsystem* LevelInstanceSubsystem = GetWorld()->GetSubsystem<ULevelInstanceSubsystem>();

			LevelInstanceSubsystem->ForEachActorInLevelInstance(LevelInstancePtr, [&](AActor* SubActor)
				{
					BottomContourActorsRawPtr.Add(SubActor);
					return true;
				});
		}
		else if (bRecursivelyAddAttachedActors)
		{
			TArray<AActor*> Meshes;
			CurrActorRawPtr->GetAttachedActors(Meshes);
			for (AActor* AttachedMesh : Meshes)
			{
				BottomContourActorsRawPtr.Add(AttachedMesh);
			}
		}
		else
		{
			BottomContourActorsRawPtr.Add(CurrActorRawPtr);
		}
	}
}

void UShallowWaterRiverComponent::AddTaggedActorsToArray(TArray<FName> &TagsToUse, TArray<AActor*>& BottomContourActorsRawPtr)
{
	// if we have a tag set
	// do an overlap test
	//  filter by tag and add to the bottomcontour actors list
	//  if a level instance is tagged, loop over the contained actors

	if (!TagsToUse.IsEmpty())
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ShallowWaterRiverActorQuery), false);

		TArray<FOverlapResult> Overlaps;
		GetWorld()->OverlapMultiByChannel(Overlaps, SystemPos, FQuat::Identity, ECollisionChannel::ECC_WorldStatic,
			FCollisionShape::MakeBox(0.5f * FVector(WorldGridSize.X, WorldGridSize.Y, 100000)), Params);

		for (const FOverlapResult& OverlapResult : Overlaps)
		{
			if (UPrimitiveComponent* PrimitiveComponent = OverlapResult.GetComponent())
			{
				if (AActor* ComponentActor = PrimitiveComponent->GetOwner())
				{
					if (TagsToUse.ContainsByPredicate([&](const FName& Tag) { return Tag == NAME_None || ComponentActor->Tags.Contains(Tag); }))
					{
						// if we have a level instance, break it up and add each actor
						if (ALevelInstance* LevelInstancePtr = Cast<ALevelInstance>(ComponentActor))
						{
							const ULevelInstanceSubsystem* LevelInstanceSubsystem = GetWorld()->GetSubsystem<ULevelInstanceSubsystem>();

							LevelInstanceSubsystem->ForEachActorInLevelInstance(LevelInstancePtr, [&](AActor* SubActor)
								{
									BottomContourActorsRawPtr.Add(SubActor);
									return true;
								});
						}
						else
						{
							BottomContourActorsRawPtr.Add(ComponentActor);
						}
					}
				}
			}
		}
	}
}

void UShallowWaterRiverComponent::Bake()
{
	EObjectFlags TextureObjectFlags = EObjectFlags::RF_Public;

	// // TODO : RiverSimSystem 대신 모든 청크의 RiverSimSystem 체크하는 함수 만들기
	// if (!RiverSimSystem || !BakedWaterSurfaceRT || !BakedFoamRT || !BakedWaterSurfaceNormalRT)
	// {
	// 	UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Bake() - No simulation to bake"));
	// 	return;
	// }

	if (RenderState != EShallowWaterRenderState::LiveSim)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Bake() - Must be in live sim mode to bake"));
		return;
	}

	if (ShallowWaterChunks.Num() == 0)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Bake() - 청크 0개"));
		return;
	}
	
	
	// // 1. 기존에 스폰했던 RVT Plane들이 있다면 깔끔하게 청소
	// for (UStaticMeshComponent* Plane : ChunkRVTPlanes)
	// {
	// 	if (Plane)
	// 	{
	// 		Plane->DestroyComponent();
	// 	}
	// }
	// ChunkRVTPlanes.Empty();
	//
	// // ... [초반 예외 처리 및 변수 초기화] ...
	// int32 TotalPhysicsPixels = SimRes.X * SimRes.Y;
	// TArray<FVector4> ShallowWaterSimArrayValues;
	// ShallowWaterSimArrayValues.SetNumZeroed(TotalPhysicsPixels);
	//
	//
	// // ------------------------------------------------------------
	// // 🚨 [추가] 마진을 잘라내기 위한 UV Scale과 Offset 계산
	// // ------------------------------------------------------------
	// // 전체 해상도 = 알맹이 해상도 + 양쪽 마진
	// float TotalResX = (float)(BaseChunkRes.X + (MarginCells * 2));
	// float TotalResY = (float)(BaseChunkRes.Y + (MarginCells * 2));
	//
	// // UV Scale: 전체 텍스처에서 알맹이가 차지하는 비율
	// float UVScaleX = (float)BaseChunkRes.X / TotalResX;
	// float UVScaleY = (float)BaseChunkRes.Y / TotalResY;
	//
	// // UV Offset: 시작점(여백)이 UV 공간(0~1)에서 어디서부터 시작하는지
	// float UVOffsetX = (float)MarginCells / TotalResX;
	// float UVOffsetY = (float)MarginCells / TotalResY;
	//
	// // 2. 각 청크를 순회하며 데이터 처리
	// for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	// {
	// 	// ==========================================
	// 	// A. CPU 물리 연산(부력)을 위한 데이터만 가볍게 추출
	// 	// ==========================================
	// 	if (Chunk.SimGridRT)
	// 	{
	// 		TArray<FFloat16Color> RawPixels;
	// 		Chunk.SimGridRT->GameThread_GetRenderTargetResource()->ReadFloat16Pixels(RawPixels);
	// 		
	// 		int32 GlobalOffsetX = Chunk.GridIndex.X * BaseChunkRes.X;
	// 		int32 GlobalOffsetY = Chunk.GridIndex.Y * BaseChunkRes.Y;
	//
	// 		for (int32 y = 0; y < BaseChunkRes.Y; ++y)
	// 		{
	// 			for (int32 x = 0; x < BaseChunkRes.X; ++x)
	// 			{
	// 				int32 ReadIdx = ((y + MarginCells) * Chunk.SimGridRT->SizeX) + (x + MarginCells);
	// 				int32 PhysicsWriteIdx = ((GlobalOffsetY + y) * SimRes.X) + (GlobalOffsetX + x);
	// 				
	// 				if (RawPixels.IsValidIndex(ReadIdx) && ShallowWaterSimArrayValues.IsValidIndex(PhysicsWriteIdx))
	// 				{
	// 					FFloat16Color P = RawPixels[ReadIdx];
	// 					ShallowWaterSimArrayValues[PhysicsWriteIdx] = FVector4(P.R, P.G, P.B, P.A);
	// 				}
	// 			}
	// 		}
	// 	}
	//
	// 	// ==========================================
	// 	// B. GPU 렌더링(RVT)을 위한 Plane 컴포넌트 스폰
	// 	// ==========================================
	// 	UStaticMeshComponent* ChunkPlane = NewObject<UStaticMeshComponent>(this);
	// 	ChunkPlane->SetStaticMesh(DefaultPlaneMesh);
	// 	ChunkPlane->SetupAttachment(this); // 현재 워터 컴포넌트에 부착
	// 	ChunkPlane->RegisterComponent();
	//
	// 	// 위치 지정 (청크의 중앙)
	// 	//ChunkPlane->SetWorldLocation(Chunk.SystemPos);
	// 	ChunkPlane->SetWorldLocation({Chunk.SystemPos.X, Chunk.SystemPos.Y, 10000});
	// 	
	// 	
	// 	// 스케일 지정 (언리얼 기본 Plane은 100x100 크기이므로, 100으로 나누어 스케일링)
	// 	ChunkPlane->SetWorldScale3D(FVector(BaseChunkSize.X / 100.0f, BaseChunkSize.Y / 100.0f, 1.0f));
	//
	// 	// 머티리얼 세팅 (개별 RT 할당)
	// 	if (RVTWriterMaterial)
	// 	{
	// 		UMaterialInstanceDynamic* PlaneMID = UMaterialInstanceDynamic::Create(RVTWriterMaterial, this);
	// 		
	// 		// 하나의 Vector(R, G, B, A) 파라미터로 묶어서 전달합니다.
	// 		PlaneMID->SetVectorParameterValue(FName("UVScaleOffset"), FLinearColor(UVScaleX, UVScaleY, UVOffsetX, UVOffsetY));
	// 		
	// 		Chunk.BakedWaterSurfaceTexture = Chunk.SimGridRT->ConstructTexture2D(this, FString::Printf(TEXT("BakedSim_%d_%d"), Chunk.GridIndex.X, Chunk.GridIndex.Y), EObjectFlags::RF_Public);
	// 		Chunk.BakedFoamTexture = Chunk.FoamRT->ConstructTexture2D(this, FString::Printf(TEXT("BakedFoam_%d_%d"), Chunk.GridIndex.X, Chunk.GridIndex.Y), EObjectFlags::RF_Public);
	// 		Chunk.BakedWaterSurfaceNormalTexture = Chunk.NormalRT->ConstructTexture2D(this, FString::Printf(TEXT("BakedNormal_%d_%d"), Chunk.GridIndex.X, Chunk.GridIndex.Y), EObjectFlags::RF_Public);
	// 		
	// 		PlaneMID->SetTextureParameterValue(FName("ChunkSimRT"), Chunk.BakedWaterSurfaceTexture);
	// 		PlaneMID->SetTextureParameterValue(FName("ChunkFoamRT"), Chunk.BakedFoamTexture);
	// 		PlaneMID->SetTextureParameterValue(FName("ChunkNormalRT"), Chunk.BakedWaterSurfaceNormalTexture);
	// 	
	// 		ChunkPlane->SetMaterial(0, PlaneMID);
	// 	}
	//
	// 	// 🚨 핵심 설정: 이 Plane은 메인 화면에 그리지 않고 RVT에만 그립니다!
	// 	//ChunkPlane->bRenderInMainPass = false;
	// 	
	// 	
	// 	if (WaterHeightRVT)
	// 	{
	// 		ChunkPlane->RuntimeVirtualTextures.Add(WaterHeightRVT);
	// 	}
	// 	if (ExtraWaterDataRVT)
	// 	{
	// 		ChunkPlane->RuntimeVirtualTextures.Add(ExtraWaterDataRVT);
	// 	}
	// 	if (WaterVelocityRVT)
	// 	{
	// 		ChunkPlane->RuntimeVirtualTextures.Add(WaterVelocityRVT);
	// 	}
	//
	// 	ChunkRVTPlanes.Add(ChunkPlane);
	// }
	//
	// // 3. 기존의 거대 UTexture2D 생성 및 InitializeVirtualTexture 호출 로직은 전부 삭제!
	//
	//
	//
	// if (BakedSim != nullptr)
	// { 
	// 	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : BakedSim->WaterBodies)
	// 	{
	// 		TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = CurrWaterBody->GetWaterBodyComponent();
	//
	// 		if (CurrWaterBodyComponent != nullptr)
	// 		{
	// 			CurrWaterBodyComponent->SetBakedShallowWaterSimulation(nullptr);
	// 			CurrWaterBodyComponent->PostEditChange();
	// 		}
	// 	}
	// }
	//
	// // 4. 물리 시뮬레이션 컴포넌트 초기화 (텍스처 포인터는 nullptr 전달)
	// // (GPU 렌더링은 RVT가 담당하므로, BakedSim은 CPU 데이터(ShallowWaterSimArrayValues)만 있으면 완벽하게 작동합니다)
	// BakedSim = NewObject<UBakedShallowWaterSimulationComponent>(this, NAME_None, RF_Public);
	// BakedSim->SimulationData = FShallowWaterSimulationGrid(ShallowWaterSimArrayValues, nullptr, FIntVector2(SimRes.X, SimRes.Y), SystemPos, WorldGridSize);
	// BakedSim->WaterBodies = AllWaterBodies;	

	////////테스트를 위해 기존 베이킹 로직 부활
	
		
	int32 TotalPixels = SimRes.X * SimRes.Y;
	
	// 3. 글로벌 텍스처 배열들 초기화
	TArray<FFloat16Color> GlobalSimPixels;
	TArray<FFloat16Color> GlobalFoamPixels;
	TArray<FFloat16Color> GlobalNormalPixels;
	
	GlobalSimPixels.SetNumZeroed(TotalPixels);
	GlobalFoamPixels.SetNumZeroed(TotalPixels);
	GlobalNormalPixels.SetNumZeroed(TotalPixels);

	//ShallowWaterSimArrayValues.Empty();
	TArray<FVector4> ShallowWaterSimArrayValues;
	ShallowWaterSimArrayValues.SetNumZeroed(TotalPixels);
	
	// -------------------------------------------------------------------
	// [람다 1: RT 마진 크롭 및 배열 병합 함수]
	// -------------------------------------------------------------------
	auto MergeChunkRT = [&](UTextureRenderTarget2D* ChunkRT, TArray<FFloat16Color>& GlobalPixels, FShallowWaterChunk& Chunk, bool bExtractPhysics)
	{
		if (!ChunkRT) return;

		TArray<FFloat16Color> RawPixels;
		ChunkRT->GameThread_GetRenderTargetResource()->ReadFloat16Pixels(RawPixels);
		int32 RTResX = ChunkRT->SizeX;

		// // 물리 데이터 배열은 메인 SimGridRT를 처리할 때 한 번만 초기화
		// if (bExtractPhysics)
		// {
		// 	Chunk.BaseResX = BaseChunkRes.X;
		// 	Chunk.BaseResY = BaseChunkRes.Y;
		// 	Chunk.BakedPhysicsData.SetNumZeroed(BaseChunkRes.X * BaseChunkRes.Y);
		// }

		int32 GlobalOffsetX = Chunk.GridIndex.X * BaseChunkRes.X;
		int32 GlobalOffsetY = Chunk.GridIndex.Y * BaseChunkRes.Y;

		for (int32 y = 0; y < BaseChunkRes.Y; ++y)
		{
			for (int32 x = 0; x < BaseChunkRes.X; ++x)
			{
				int32 ReadX = x + MarginCells;
				int32 ReadY = y + MarginCells;
				int32 ReadIdx = (ReadY * RTResX) + ReadX;

				if (RawPixels.IsValidIndex(ReadIdx))
				{
					FFloat16Color PixelColor = RawPixels[ReadIdx];

					
					int32 GlobalWriteIdx = ((GlobalOffsetY + y) * SimRes.X) + (GlobalOffsetX + x);

					if (GlobalPixels.IsValidIndex(GlobalWriteIdx))
					{
						GlobalPixels[GlobalWriteIdx] = PixelColor;

						// SimGrid일 때만 CPU 배열의 정확한 제 위치(GlobalWriteIdx)에 물리 데이터 삽입
						if (bExtractPhysics)
						{
							ShallowWaterSimArrayValues[GlobalWriteIdx] = FVector4(PixelColor.R, PixelColor.G, PixelColor.B, PixelColor.A);
						}
					}
				}
			}
		}
	};

	// -------------------------------------------------------------------
	// [람다 2: 병합된 픽셀 배열로 UTexture2D 생성 함수]
	// -------------------------------------------------------------------
	auto CreateTexture = [&](const TArray<FFloat16Color>& PixelData, const FString& TexName) -> UTexture2D*
	{
		UTexture2D* NewTexture = NewObject<UTexture2D>(this, FName(*TexName), RF_Public
			);
		NewTexture->Source.Init(SimRes.X, SimRes.Y, 1, 1, TSF_RGBA16F, (uint8*)PixelData.GetData());
		NewTexture->SRGB = false;
		NewTexture->CompressionSettings = TC_HDR;
		NewTexture->Filter = TF_Bilinear;
		
		// 외곽쪽에 물이 있을 경우 생기는 수면 높이 이상 현상 방지.
		NewTexture->AddressX = TA_Clamp;
		NewTexture->AddressY = TA_Clamp;
		
		NewTexture->UpdateResource();
		return NewTexture;
	};

	// 4. 각 청크를 순회하며 3가지 렌더 타깃 모두 병합 처리
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		// (주의: Chunk 구조체에 FoamRT와 NormalRT가 선언되어 있어야 함)
		MergeChunkRT(Chunk.SimGridRT, GlobalSimPixels, Chunk, true);   // Sim 데이터 병합 및 물리 데이터 추출
		MergeChunkRT(Chunk.FoamRT, GlobalFoamPixels, Chunk, false);    // 거품 데이터 병합
		MergeChunkRT(Chunk.NormalRT, GlobalNormalPixels, Chunk, false); // 노말 데이터 병합
	}

	// // 외곽선 0 초기화 (Clamp 늘어짐 방지)
	// for (int32 y = 0; y < SimRes.Y; ++y)
	// {
	// 	for (int32 x = 0; x < SimRes.X; ++x)
	// 	{
	// 		if (x == 0 || x == SimRes.X - 1 || y == 0 || y == SimRes.Y - 1)
	// 		{
	// 			int32 EdgeIdx = (y * SimRes.X) + x;
	// 			
	// 			// 모든 물리 데이터를 0(수심 0, 높이 0)으로 덮어씀 -> 수심이 0이여서 아마 문제 없겠지만 높이가 0인건 문제의 여지가 있음
	// 			GlobalSimPixels[EdgeIdx] = FFloat16Color({0.f, 0.f, 0.f, 0.f});
	// 			GlobalFoamPixels[EdgeIdx] = FFloat16Color({0.f, 0.f, 0.f, 0.f});
	// 			GlobalNormalPixels[EdgeIdx] = FFloat16Color({0.f, 0.f, 0.f, 0.f});
	// 			ShallowWaterSimArrayValues[EdgeIdx] = FVector4(0.f, 0.f, 0.f, 0.f);
	// 		}
	// 	}
	// }
	
	// 외곽에 물 벽이 생기는 걸 막기 위해 외곽 몇 픽셀은 물 깊이 0으로 만들기.
	// TODO : 몇픽셀이라도 잘리는게 좀 그래. 뭐 엄청 여유 있으면 아예 크기 자체를 늘려서 마진을 만드는 것도 괜찮을지도?
	int32 DepthMarginSize = 8; 

	for (int32 y = 0; y < SimRes.Y; ++y)
	{
		for (int32 x = 0; x < SimRes.X; ++x)
		{
			// 마진 구역(최외곽 테두리)인지 확인
			if (x < DepthMarginSize || x >= SimRes.X - DepthMarginSize || y < DepthMarginSize || y >= SimRes.Y - DepthMarginSize)
			{
				int32 EdgeIdx = (y * SimRes.X) + x;
			
				// 1. 가장 가까운 '안전한 안쪽 픽셀'의 좌표를 구합니다.
				int32 InnerX = FMath::Clamp(x, DepthMarginSize, SimRes.X - DepthMarginSize - 1);
				int32 InnerY = FMath::Clamp(y, DepthMarginSize, SimRes.Y - DepthMarginSize - 1);
				int32 InnerIdx = (InnerY * SimRes.X) + InnerX;

				// 2. 안쪽 픽셀의 높이(R 채널) 값을 가져옵니다.
				float SafeHeight = GlobalSimPixels[InnerIdx].R.GetFloat();

				// 3. 💡 핵심: 높이(R)는 복사해서 보존하고, 수심(G)과 유속(B, A)만 0으로 덮어씁니다!
				GlobalSimPixels[EdgeIdx] = FFloat16Color({SafeHeight, 0.f, 0.f, 0.f});
				GlobalFoamPixels[EdgeIdx] = FFloat16Color({0.f, 0.f, 0.f, 0.f});
			
				// 노말맵은 위를 향하도록 (0, 0, 1) 평탄화
				GlobalNormalPixels[EdgeIdx] = FFloat16Color({0.f, 0.f, 1.f, 0.f}); 
			
				ShallowWaterSimArrayValues[EdgeIdx] = FVector4(SafeHeight, 0.f, 0.f, 0.f);
			}
		}
	}
	
	
	// 5. 람다를 사용해 3개의 텍스처 에셋 일괄 생성
	BakedWaterSurfaceTexture = CreateTexture(GlobalSimPixels, TEXT("BakedRiverTexture"));
	BakedFoamTexture = CreateTexture(GlobalFoamPixels, TEXT("BakedFoamTexture"));
	BakedWaterSurfaceNormalTexture = CreateTexture(GlobalNormalPixels, TEXT("BakedNormalTexture"));
	
	if (bUseVirtualTextures)
	{
		InitializeVirtualTexture(BakedWaterSurfaceTexture);
	}
	
	
	if (bUseVirtualTextures)
	{		
		InitializeVirtualTexture(BakedFoamTexture);
	}
	
	if (bUseVirtualTextures)
	{
		InitializeVirtualTexture(BakedWaterSurfaceNormalTexture);
	}
	
	if (BakedSim != nullptr)
	{ 
		for (TSoftObjectPtr<AWaterBody > CurrWaterBody : BakedSim->WaterBodies)
		{
			TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = CurrWaterBody->GetWaterBodyComponent();

			if (CurrWaterBodyComponent != nullptr)
			{
				CurrWaterBodyComponent->SetBakedShallowWaterSimulation(nullptr);
				CurrWaterBodyComponent->PostEditChange();
			}
		}
	}

	BakedSim = NewObject<UBakedShallowWaterSimulationComponent>(this, NAME_None, RF_Public);
	BakedSim->SimulationData = FShallowWaterSimulationGrid(ShallowWaterSimArrayValues, BakedWaterSurfaceTexture, FIntVector2(SimRes.X, SimRes.Y), SystemPos, WorldGridSize);
	BakedSim->WaterBodies = AllWaterBodies;	


	
	
	////////////////////////////////////////
	
	
	
	
	TMap<FKConvexElem*, float> ConvexToMaxHeight;
	for (int32 y = 0; y < SimRes.Y; ++y) {
	for (int32 x = 0; x < SimRes.X; ++x) {
		FVector WorldPos = BakedSim->SimulationData.IndexToWorld(FIntVector2(x, y));

		FVector Vel;
		float Height, Depth;
		BakedSim->SimulationData.QueryShallowWaterSimulationAtIndex(FIntVector2(x, y), Vel, Height, Depth);
		WorldPos.Z = Height;
				
		if (Depth > 1e-5)
		{
			for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
			{		
				////////이따가 삭제 - 임시로 Lake도 bake에 포함하기 위해 추가.
				if (Cast<AWaterBodyLake>(CurrWaterBody.Get()))
				{
					UE_LOG(LogTemp, Warning, TEXT("CurrWaterBody: WaterBodyLake 피카츄"));
					continue;
					
				}
				//~~~~이따가 삭제
				
				TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = CurrWaterBody->GetWaterBodyComponent();

				TArray<UPrimitiveComponent*> CollisionComponents = CurrWaterBodyComponent->GetCollisionComponents();
				
				
				
				for (UPrimitiveComponent* CurrCollisionComponent : CollisionComponents)
				{
					USplineMeshComponent* CurrSplineComponent = StaticCast<USplineMeshComponent*>(CurrCollisionComponent);
					TObjectPtr<UBodySetup> CurrBodySetup = CurrSplineComponent->BodySetup;

					const FTransform CurrMeshTransform = CurrCollisionComponent->GetComponentTransform();

					// make sure the collision convex hull vertices are clamped to the min/max water height
					for (FKConvexElem& ConvexElem : CurrBodySetup->AggGeom.ConvexElems)
					{					
						const TArray<FVector>& VertexData = ConvexElem.VertexData;		

						// see if the current point is inside the convex projected to the xy plane
						const FBox CurrBox = ConvexElem.CalcAABB(CurrMeshTransform, FVector(1, 1, 1));										

						if (CurrBox.IsInsideXY(FBox(WorldPos, WorldPos)))
						{						
							float* TmpMaxHeight = ConvexToMaxHeight.Find(&ConvexElem);
							if (TmpMaxHeight == nullptr)
							{							
								ConvexToMaxHeight.Emplace(&ConvexElem, WorldPos.Z);
							}
							else
							{
								*TmpMaxHeight = FMath::Max(*TmpMaxHeight, WorldPos.Z);
							}
						}
					}
				}
			}
		}
	}}
	
	
	// set the sim texture on each water body that is in the simulated river.  
	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
	{
		////////이따가 삭제2 - 임시로 Lake도 bake에 포함하기 위해 추가.
		if (Cast<AWaterBodyLake>(CurrWaterBody.Get()))
		{
			UE_LOG(LogTemp, Warning, TEXT("CurrWaterBody: WaterBodyLake 라이츄"));
			continue;
		}
		//~~~~이따가 삭제
		
		TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = CurrWaterBody->GetWaterBodyComponent();
				
		CurrWaterBodyComponent->SetBakedShallowWaterSimulation(BakedSim);

		// grow bounds in z to include the tallest height
		TArray<UPrimitiveComponent*> CollisionComponents = CurrWaterBodyComponent->GetCollisionComponents();
		
		// Make sure that the collision objects includes the maximum height of the baked water sim otherwise
		// we will miss collisions.
		for (UPrimitiveComponent* CurrCollisionComponent : CollisionComponents)
		{
			USplineMeshComponent* CurrSplineComponent = StaticCast<USplineMeshComponent*>(CurrCollisionComponent);
			
			TObjectPtr<UBodySetup> CurrBodySetup = CurrSplineComponent->BodySetup;

			FTransform CurrMeshTransform = CurrCollisionComponent->GetComponentTransform();

			// make sure the collision convex hull vertices are clamped to the min/max water height
			for (FKConvexElem& ConvexElem : CurrBodySetup->AggGeom.ConvexElems)
			{
				TArray<FVector>& VertexData = ConvexElem.VertexData;

				if (const float* WorldMaxZForConvex = ConvexToMaxHeight.Find(&ConvexElem))
				{
					// for each vertex in the convex hull, set the Z to the maximum baked water sim Z height for the convex
					int32 Idx = 0;
					for (FVector& Vertex : VertexData)
					{
						// only top vertices are 4,5,6,7
						if (Idx >= 4)
						{
							FVector VWorld = CurrMeshTransform.TransformPosition(Vertex);

							VWorld.Z = FMath::Max(VWorld.Z, *WorldMaxZForConvex);

							const FVector VLocal = CurrMeshTransform.InverseTransformPosition(VWorld);
							Vertex.X = VLocal.X;
							Vertex.Y = VLocal.Y;
							Vertex.Z = VLocal.Z;
						}


						#if ENABLE_DRAW_DEBUG
						if (bShallowWaterRiverDebugVisualize)
						{		
							FVector VWorld = CurrMeshTransform.TransformPosition(Vertex);

							switch (Idx)
							{
								case 0:
								DrawDebugSphere(GetWorld() , VWorld, 10., 2, FColor::Red, true);		
								break;
								case 1:
								DrawDebugSphere(GetWorld() , VWorld, 10., 3, FColor::Green, true);		
								break;
								case 2:
								DrawDebugSphere(GetWorld() , VWorld, 10., 4, FColor::Blue, true);		
								break;
								case 3:
								DrawDebugSphere(GetWorld() , VWorld, 10., 5, FColor::Black, true);		
								break;
								case 4:
								DrawDebugSphere(GetWorld() , VWorld, 10., 6, FColor::White, true);	//	
								break;
								case 5:
								DrawDebugSphere(GetWorld() , VWorld, 10., 7, FColor::Magenta, true); //		
								break;
								case 6:
								DrawDebugSphere(GetWorld() , VWorld, 10., 8, FColor::Orange, true);	//	
								break;
								case 7:
								DrawDebugSphere(GetWorld() , VWorld, 10., 9, FColor::Purple, true);	//	
								break;								
							}
							
						}
						#endif
												
						Idx++;
					}
				}
			}			

			CurrSplineComponent->PostEditChange();
		}
		
		
		CurrWaterBodyComponent->PostEditChange();
	}
}

void UShallowWaterRiverComponent::InitializeVirtualTexture(TObjectPtr<UTexture2D> InTexture)
{	
	InTexture->Modify();
	InTexture->MipGenSettings = TextureMipGenSettings::TMGS_SimpleAverage;
	InTexture->PowerOfTwoMode = ETexturePowerOfTwoSetting::PadToPowerOfTwo;
	InTexture->VirtualTextureStreaming = true;

	InTexture->UpdateResource();	
	InTexture->WaitForStreaming(true, true);	
	InTexture->BlockOnAnyAsyncBuild();
	InTexture->PostEditChange();
}

bool UShallowWaterRiverComponent::QueryWaterAtSplinePoint(TSoftObjectPtr<AWaterBody> WaterBody, int SplinePoint, FVector& OutPos, FVector& OutTangent, float& OutWidth, float& OutDepth)
{	
	if (WaterBody != nullptr)
	{
		TObjectPtr<UWaterBodyComponent> CurrWaterBodyComponent = WaterBody->GetWaterBodyComponent();

		UWaterSplineComponent* CurrSpline = WaterBody->GetWaterSpline();
		
		if (CurrSpline != nullptr)
		{
			// -1 means last spline point
			if (SplinePoint == -1)
			{
				SplinePoint = CurrSpline->GetNumberOfSplinePoints() - 1;
			}

			UWaterSplineMetadata* Metadata = WaterBody->GetWaterSplineMetadata();

			if (Metadata != nullptr)
			{
				OutPos = CurrSpline->GetLocationAtSplineInputKey(SplinePoint, ESplineCoordinateSpace::Local);
				OutPos = CurrSpline->GetComponentTransform().TransformPosition(OutPos);

				OutWidth = Metadata->RiverWidth.Points[SplinePoint].OutVal;
				OutDepth = Metadata->Depth.Points[SplinePoint].OutVal;

				OutTangent = CurrSpline->GetLeaveTangentAtSplinePoint(SplinePoint, ESplineCoordinateSpace::Local);

				OutTangent = CurrSpline->GetComponentTransform().TransformVector(OutTangent);
				OutTangent.Normalize();
			}
			else
			{
				UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::QueryWaterAtSplinePoint() - Water spline metadata is null"));
				return false;
			}
		}
		else
		{
			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::QueryWaterAtSplinePoint() - Water spline component is null"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::QueryWaterAtSplinePoint() - Water actor is null"));
		return false;
	}

	return true;
}

void UShallowWaterRiverComponent::OnWaterInfoTextureArrayCreated(const UTextureRenderTarget2DArray* InWaterInfoTexture)
{	
	if (InWaterInfoTexture == nullptr)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::OnWaterInfoTextureCreated was called with NULL WaterInfoTexture"));
		return;
	}
	
	WaterInfoTexture = InWaterInfoTexture;
	
	// 수정
	
	// if (RiverSimSystem)
	// {
	// 	UTexture* WITTextureArray = Cast<UTexture>(const_cast<UTextureRenderTarget2DArray*>(WaterInfoTexture.Get()));
	// 	if (WITTextureArray == nullptr)
	// 	{
	// 		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::OnWaterInfoTextureCreated was called with Water Info Texture that isn't valid"));
	// 		return;
	// 	}
	//
	// 	RiverSimSystem->SetVariableTexture(FName("WaterInfoTexture"), WITTextureArray);
	// }
	// else
	// {
	// 	UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::OnWaterInfoTextureCreated was called with NULL ShallowWaterNiagaraSimulation"));
	// 	return;
	// }
	
	UTexture* WITTextureArray = Cast<UTexture>(const_cast<UTextureRenderTarget2DArray*>(WaterInfoTexture.Get()));
	if (WITTextureArray == nullptr)
	{
		UE_LOG(LogShallowWater, Warning,
			   TEXT(
				   "UShallowWaterRiverComponent::OnWaterInfoTextureCreated was called with Water Info Texture that isn't valid"
			   ));
		return;
	}
	
	for (FShallowWaterChunk WaterChunk : ShallowWaterChunks)
	{
		if (WaterChunk.RiverSimSystem)
		{
			WaterChunk.RiverSimSystem->SetVariableTexture(FName("WaterInfoTexture"), WITTextureArray);
		}
	}
	/// 수정 끝
}
#endif

void UShallowWaterRiverComponent::SetPaused(bool Pause)
{
	// if (RiverSimSystem)
	// {
	// 	RiverSimSystem->SetPaused(Pause);
	// }
	for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
	{
		if (Chunk.RiverSimSystem)
		{
			Chunk.RiverSimSystem->SetPaused(Pause);
		}
	}

	UFFTOceanPatchSubsystem *OceanPatchSubsystem = GetWorld()->GetSubsystem<UFFTOceanPatchSubsystem>();
	if (OceanPatchSubsystem != nullptr)
	{
		TObjectPtr<UNiagaraComponent> OceanSystem = OceanPatchSubsystem->GetOceanSystem();
		if (OceanSystem)
		{
			OceanSystem->SetPaused(Pause);
		}
	}
}

void UShallowWaterRiverComponent::UpdateRenderState()
{
	TObjectPtr<UTextureRenderTarget2D>  OceanPatchNormalRT = GetSharedFFTOceanPatchNormalRTFromSubsystem(GetWorld());

	if (OceanPatchNormalRT == nullptr)
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::Rebuild() - ocean patch normal RT is not initialized"));
		return;
	}
	
	NormalDetailRT = OceanPatchNormalRT;

	if (BakedSimMaterial == nullptr)
	{
		BakedSimMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/WaterAdvanced/Niagara/Materials/SW_Water_Material_River.SW_Water_Material_River"));
	}
	
	if (BakedSimRiverToLakeTransitionMaterial == nullptr)
	{
		BakedSimRiverToLakeTransitionMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/WaterAdvanced/Niagara/Materials/SW_Water_Material_River_To_Lake_Transition.SW_Water_Material_River_To_Lake_Transition"));
	}

	if (BakedSimRiverToOceanTransitionMaterial == nullptr)
	{
		BakedSimRiverToOceanTransitionMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/WaterAdvanced/Niagara/Materials/SW_Water_Material_River_To_Ocean_Transition.SW_Water_Material_River_To_Ocean_Transition"));
	}
	
	if (SplineRiverMaterial == nullptr)
	{
		SplineRiverMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/WaterAdvanced/Niagara/Materials/SW_Water_Material_River_Spline.SW_Water_Material_River_Spline"));
	}

	if (SplineRiverToLakeTransitionMaterial == nullptr)
	{
		SplineRiverToLakeTransitionMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/WaterAdvanced/Niagara/Materials/SW_Water_Material_River_To_Lake_Transition_Spline.SW_Water_Material_River_To_Lake_Transition_Spline"));
	}

	if (SplineRiverToOceanTransitionMaterial == nullptr)
	{
		SplineRiverToOceanTransitionMaterial = LoadObject<UMaterialInstance>(nullptr, TEXT("/WaterAdvanced/Niagara/Materials/SW_Water_Material_River_To_Ocean_Transition_Spline.SW_Water_Material_River_To_Ocean_Transition_Spline"));
	}

	bool bReadBakedSim = RenderState == EShallowWaterRenderState::BakedSim || RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::WaterComponent;
	bool RenderWaterBody = RenderState == EShallowWaterRenderState::WaterComponent || RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim;
	bool RenderSecondary = 
		RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || 
		RenderState == EShallowWaterRenderState::BakedSim || RenderState == EShallowWaterRenderState::LiveSim;

	// if (RiverSimSystem != nullptr)
	// {		
	// 	RiverSimSystem->SetVariableVec2(FName("SimRes"), SimRes);
	//
	// 	RiverSimSystem->SetVariableBool(FName("RenderWaterSurface"), !RenderWaterBody);
	// 	RiverSimSystem->SetVariableBool(FName("RenderSecondary"), RenderSecondary);
	// 	RiverSimSystem->SetVariableBool(FName("DebugRenderBottomContour"), RenderState == EShallowWaterRenderState::DebugRenderBottomContour);
	// 	RiverSimSystem->SetVariableBool(FName("DebugRenderFoam"), RenderState == EShallowWaterRenderState::DebugRenderFoam);
	// 	RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
	// 	
	// 	RiverSimSystem->SetVariableTextureRenderTarget("OceanNormalRT", NormalDetailRT);
	// 	RiverSimSystem->ReinitializeSystem();
	// }
	
	if (bReadBakedSim)
	{
		//////////////
		for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
		{
			// 1. 개별 청크 시뮬레이션 시스템 종료
			if (Chunk.RiverSimSystem)
			{
				Chunk.RiverSimSystem->DeactivateImmediate();
			}
		}
		
		// 2. 마스터 시스템 활성화 및 거품 텍스처 배정
		if (RiverSimSystem)
		{
			if (BakedWaterSurfaceTexture != nullptr && BakedFoamTexture != nullptr && BakedWaterSurfaceNormalTexture != nullptr)
			{
				RiverSimSystem->SetVariableTexture(FName("BakedSimTexture"), BakedWaterSurfaceTexture);
				RiverSimSystem->SetVariableTexture(FName("BakedFoamTexture"), BakedFoamTexture);
				RiverSimSystem->SetVariableTexture(FName("BakedWaterSurfaceNormalTexture"), BakedWaterSurfaceNormalTexture);
			}
			
			RiverSimSystem->SetVariableVec2(FName("WorldGridSize"), WorldGridSize);
			RiverSimSystem->SetVariableInt(FName("ResolutionMaxAxis"), ResolutionMaxAxis);
			RiverSimSystem->SetVariableVec2(FName("SimRes"), SimRes);

			RiverSimSystem->SetVariableBool(FName("RenderWaterSurface"), !RenderWaterBody);
			RiverSimSystem->SetVariableBool(FName("RenderSecondary"), RenderSecondary);
			RiverSimSystem->SetVariableBool(FName("DebugRenderBottomContour"), RenderState == EShallowWaterRenderState::DebugRenderBottomContour);
			RiverSimSystem->SetVariableBool(FName("DebugRenderFoam"), RenderState == EShallowWaterRenderState::DebugRenderFoam);
			RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);
		
			RiverSimSystem->SetVariableTextureRenderTarget("OceanNormalRT", NormalDetailRT);
			
			RiverSimSystem->ReinitializeSystem();
			
			
		}
	}
	else
	{
		// TODO 여기에 RiverSImSystem 안보이게도 해야해. 여기 말고 Rebuild에서 해야하나?
		
		for (FShallowWaterChunk& Chunk : ShallowWaterChunks)
		{
			int32 ChunkIndex = Chunk.GridIndex.Y * ChunkGridDimensions.X + Chunk.GridIndex.X;

			if (Chunk.RiverSimSystem != nullptr)
			{
				Chunk.RiverSimSystem->SetVariableVec2(FName("SimRes"), SimRes);

				Chunk.RiverSimSystem->SetVariableBool(FName("RenderWaterSurface"), !RenderWaterBody);
				Chunk.RiverSimSystem->SetVariableBool(FName("RenderSecondary"), RenderSecondary);

				Chunk.RiverSimSystem->SetVariableBool(FName("DebugRenderBottomContour"),
				                                      RenderState ==
				                                      EShallowWaterRenderState::DebugRenderBottomContour);
				Chunk.RiverSimSystem->SetVariableBool(FName("DebugRenderFoam"),
				                                      RenderState == EShallowWaterRenderState::DebugRenderFoam);

				Chunk.RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), bReadBakedSim);

				// Chunk.RiverSimSystem->SetVariableBool(FName("ReadCachedSim"), ChunkIndex != CurrentTargetChunkIndex);//
				// Chunk.RiverSimSystem->SetVariableBool(FName("RenderSecondary"), ChunkIndex != CurrentTargetChunkIndex);//

				Chunk.RiverSimSystem->SetVariableTextureRenderTarget("OceanNormalRT", NormalDetailRT);
				//Chunk.RiverSimSystem->ReinitializeSystem();
				Chunk.RiverSimSystem->DestroyInstanceNotComponent();
			}
		}
	}
	
	

	if ((RenderState == EShallowWaterRenderState::BakedSim || RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim) && 
		(BakedWaterSurfaceTexture == nullptr || BakedWaterSurfaceTexture->GetSizeX() == 0 || BakedWaterSurfaceTexture->GetSizeY() == 0))
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::UpdateRenderState() - No baked sim to render"));		
	}

	for (TSoftObjectPtr<AWaterBody > CurrWaterBody : AllWaterBodies)
	{
		if (!CurrWaterBody)
		{
			UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::UpdateRenderState() - Water Body Actor is null- skipping setting render state"));
			continue;
		}

		TObjectPtr<UWaterBodyRiverComponent> CurrWaterBodyComponent = Cast<UWaterBodyRiverComponent>(CurrWaterBody->GetWaterBodyComponent());

		if (CurrWaterBodyComponent != nullptr)
		{
			CurrWaterBodyComponent->SetVisibility(RenderWaterBody);

			if (RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim)
			{				
				CurrWaterBodyComponent->SetWaterMaterial(BakedSimMaterial);
				UMaterialInstanceDynamic* WaterMID = CurrWaterBodyComponent->GetWaterMaterialInstance();			
				SetWaterMIDParameters(WaterMID);

				CurrWaterBodyComponent->SetLakeTransitionMaterial(BakedSimRiverToLakeTransitionMaterial);
				UMaterialInstanceDynamic* WaterLakeTransitionMID = CurrWaterBodyComponent->GetRiverToLakeTransitionMaterialInstance();
				SetWaterMIDParameters(WaterLakeTransitionMID);

				CurrWaterBodyComponent->SetOceanTransitionMaterial(BakedSimRiverToOceanTransitionMaterial);
				UMaterialInstanceDynamic* WaterOceanTransitionMID = CurrWaterBodyComponent->GetRiverToOceanTransitionMaterialInstance();
				SetWaterMIDParameters(WaterOceanTransitionMID);
												
				UMaterialInstanceDynamic* WaterInfoMID = CurrWaterBodyComponent->GetWaterInfoMaterialInstance();
				if (WaterInfoMID)
				{
					WaterInfoMID->SetTextureParameterValue("BakedWaterSimTex", BakedWaterSurfaceTexture);
					WaterInfoMID->SetTextureParameterValue("FoamTex", BakedFoamTexture);
					WaterInfoMID->SetTextureParameterValue("BakedWaterSimNormalTex", BakedWaterSurfaceNormalTexture);
					WaterInfoMID->SetVectorParameterValue("BakedWaterSimLocation", SystemPos);
					WaterInfoMID->SetDoubleVectorParameterValue("BakedWaterSimLocationDouble", SystemPos);
					WaterInfoMID->SetVectorParameterValue("BakedWaterSimSize", FVector(WorldGridSize.X, WorldGridSize.Y, 1));
				}
				else
				{
					UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::UpdateRenderState() - Water Component Water Info MID is null"));
					return;
				}
			}
			else if (RenderState == EShallowWaterRenderState::WaterComponent)
			{
				CurrWaterBodyComponent->SetWaterMaterial(SplineRiverMaterial);
				CurrWaterBodyComponent->SetLakeTransitionMaterial(SplineRiverToLakeTransitionMaterial);
				CurrWaterBodyComponent->SetOceanTransitionMaterial(SplineRiverToOceanTransitionMaterial);
			}

			CurrWaterBodyComponent->SetUseBakedSimulationForQueriesAndPhysics(
				RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::BakedSim);

			/*
			// #todo(dmp): I'd prefer if we could set an editor time only static switch to control using baked sims in the material or not
			TArray<FMaterialParameterInfo> OutMaterialParameterInfos;
			TArray<FGuid> Guids;
			WaterMID->GetAllStaticSwitchParameterInfo(OutMaterialParameterInfos, Guids);

			for (FMaterialParameterInfo& MaterialParameterInfo : OutMaterialParameterInfos)
			{
				if (MaterialParameterInfo.Name == "UseBakedSim")
				{
					WaterMID->SetStaticSwitchParameterValueEditorOnly(MaterialParameterInfo, RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim);
				}
			}
			*/			
		}
		
		//// 나중에 삭제 3
		TObjectPtr<UWaterBodyLakeComponent> CurrWaterBodyComponent2 = Cast<UWaterBodyLakeComponent>(CurrWaterBody->GetWaterBodyComponent());
		if (CurrWaterBodyComponent2 != nullptr)
		{
			CurrWaterBodyComponent2->SetVisibility(RenderWaterBody);

			if (RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim)
			{				
				//CurrWaterBodyComponent2->SetWaterMaterial(BakedSimMaterial);
				CurrWaterBodyComponent2->SetWaterMaterial(MyTestLakeMaterial);
				UMaterialInstanceDynamic* WaterMID = CurrWaterBodyComponent->GetWaterMaterialInstance();			
				SetWaterMIDParameters(WaterMID);
				
												
				UMaterialInstanceDynamic* WaterInfoMID = CurrWaterBodyComponent2->GetWaterInfoMaterialInstance();
				if (WaterInfoMID)
				{
					WaterInfoMID->SetTextureParameterValue("BakedWaterSimTex", BakedWaterSurfaceTexture);
					WaterInfoMID->SetTextureParameterValue("FoamTex", BakedFoamTexture);
					WaterInfoMID->SetTextureParameterValue("BakedWaterSimNormalTex", BakedWaterSurfaceNormalTexture);
					WaterInfoMID->SetVectorParameterValue("BakedWaterSimLocation", SystemPos);
					WaterInfoMID->SetDoubleVectorParameterValue("BakedWaterSimLocationDouble", SystemPos);
					WaterInfoMID->SetVectorParameterValue("BakedWaterSimSize", FVector(WorldGridSize.X, WorldGridSize.Y, 1));
				}
				else
				{
					UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::UpdateRenderState() - Water Component Water Info MID is null"));
					return;
				}
			}
			else if (RenderState == EShallowWaterRenderState::WaterComponent)
			{
				CurrWaterBodyComponent2->SetWaterMaterial(SplineRiverMaterial);
			}

			CurrWaterBodyComponent2->SetUseBakedSimulationForQueriesAndPhysics(
				RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim || RenderState == EShallowWaterRenderState::BakedSim);

			/*
			// #todo(dmp): I'd prefer if we could set an editor time only static switch to control using baked sims in the material or not
			TArray<FMaterialParameterInfo> OutMaterialParameterInfos;
			TArray<FGuid> Guids;
			WaterMID->GetAllStaticSwitchParameterInfo(OutMaterialParameterInfos, Guids);

			for (FMaterialParameterInfo& MaterialParameterInfo : OutMaterialParameterInfos)
			{
				if (MaterialParameterInfo.Name == "UseBakedSim")
				{
					WaterMID->SetStaticSwitchParameterValueEditorOnly(MaterialParameterInfo, RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim);
				}
			}
			*/			
		}
		/// ~~~~~~나중에 삭제3
	}
	
	
	
	

	bRenderStateTickInitialize = true;
}

void UShallowWaterRiverComponent::SetWaterMIDParameters(UMaterialInstanceDynamic* WaterMID)
{
	if (WaterMID)
	{		
		if (bUseVirtualTextures)
		{
			WaterMID->SetTextureParameterValue("BakedWaterSimTexVT", BakedWaterSurfaceTexture);
			WaterMID->SetTextureParameterValue("FoamTexVT", BakedFoamTexture);
			WaterMID->SetTextureParameterValue("BakedWaterSimNormalTexVT", BakedWaterSurfaceNormalTexture);

			UTexture2D *EmptyPtr = nullptr;
			WaterMID->SetTextureParameterValue("BakedWaterSimTex", EmptyPtr);
			WaterMID->SetTextureParameterValue("FoamTex", EmptyPtr);
			WaterMID->SetTextureParameterValue("BakedWaterSimNormalTex", EmptyPtr);
		}
		else
		{
			WaterMID->SetTextureParameterValue("BakedWaterSimTex", BakedWaterSurfaceTexture);
			WaterMID->SetTextureParameterValue("FoamTex", BakedFoamTexture);
			WaterMID->SetTextureParameterValue("BakedWaterSimNormalTex", BakedWaterSurfaceNormalTexture);

			UTexture2D *EmptyPtr = nullptr;
			WaterMID->SetTextureParameterValue("BakedWaterSimTexVT", EmptyPtr);
			WaterMID->SetTextureParameterValue("FoamTexVT", EmptyPtr);
			WaterMID->SetTextureParameterValue("BakedWaterSimNormalTexVT", EmptyPtr);
		}


		WaterMID->SetVectorParameterValue("BakedWaterSimLocation", SystemPos);
		WaterMID->SetDoubleVectorParameterValue("BakedWaterSimLocationDouble", SystemPos);
		WaterMID->SetVectorParameterValue("BakedWaterSimSize", FVector(WorldGridSize.X, WorldGridSize.Y, 1));

		WaterMID->SetTextureParameterValue("NormalDetailTex", NormalDetailRT);

		WaterMID->SetScalarParameterValue("BakedWaterSimDx", SimDx);

		WaterMID->SetScalarParameterValue("UseBakedSimHack", RenderState == EShallowWaterRenderState::WaterComponentWithBakedSim ? 1 : 0);

		WaterMID->SetVectorParameterValue("SimRes", FVector(SimRes.X, SimRes.Y, 0));
	}
	else
	{
		UE_LOG(LogShallowWater, Warning, TEXT("UShallowWaterRiverComponent::UpdateRenderState() - Water Component MID is null"));
		return;
	}
}

AShallowWaterRiver::AShallowWaterRiver(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ShallowWaterRiverComponent = CreateDefaultSubobject<UShallowWaterRiverComponent>(TEXT("ShallowWaterRiverComponent"));
	RootComponent = ShallowWaterRiverComponent;

	PrimaryActorTick.bCanEverTick = true;
	SetHidden(false);
}

