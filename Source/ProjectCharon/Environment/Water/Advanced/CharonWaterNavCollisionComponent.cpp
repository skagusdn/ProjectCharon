// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonWaterNavCollisionComponent.h"

#include "Logging.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveViewRelevance.h"
#include "SceneManagement.h"
#include "AI/Navigation/NavAreaBase.h"
#include "AI/Navigation/NavigationRelevantData.h"
#include "AI/NavigationSystemHelpers.h"
#include "PhysicsEngine/BodySetup.h"

namespace CharonWaterNavCollision
{
	// 콜리전 컨벡스의 외곽선을 에디터 뷰포트에 그리는 씬 프록시.
	// 일반 UPrimitiveComponent에는 렌더링 수단이 없어서, 이게 없으면 콜리전이 있어도 화면에 보이지 않는다.
	class FNavCollisionSceneProxy final : public FPrimitiveSceneProxy
	{
	public:

		explicit FNavCollisionSceneProxy(UCharonWaterNavCollisionComponent* Component)
			: FPrimitiveSceneProxy(Component)
		{
			bWillEverBeLit = false;

			const UBodySetup* BodySetup = Component->GetBodySetup();
			if (BodySetup == nullptr)
			{
				return;
			}

			// 렌더 스레드로 넘기기 전에 로컬 공간 선분 목록으로 만들어 둔다.
			for (const FKConvexElem& Convex : BodySetup->AggGeom.ConvexElems)
			{
				const TArray<FVector>& Verts = Convex.VertexData;

				if (Verts.Num() == 8)
				{
					// 강 컨벡스는 바닥 0~3, 윗면 4~7. 정점 순서를 알 수 없어 각 면의 모든 쌍을 잇는다.
					for (int32 i = 0; i < 4; ++i)
					{
						Lines.Emplace(Verts[i], Verts[i + 4]);
						for (int32 j = i + 1; j < 4; ++j)
						{
							Lines.Emplace(Verts[i], Verts[j]);
							Lines.Emplace(Verts[i + 4], Verts[j + 4]);
						}
					}
				}
				else
				{
					for (int32 i = 0; i < Verts.Num(); ++i)
					{
						for (int32 j = i + 1; j < Verts.Num(); ++j)
						{
							Lines.Emplace(Verts[i], Verts[j]);
						}
					}
				}
			}
		}

		virtual SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
			uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			const FMatrix& ProxyLocalToWorld = GetLocalToWorld();
			const FLinearColor LineColor(0.f, 1.f, 1.f);
			constexpr float LineThickness = 2.f;

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
			{
				if (!(VisibilityMap & (1 << ViewIndex)))
				{
					continue;
				}

				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				for (const TPair<FVector, FVector>& Line : Lines)
				{
					PDI->DrawLine(ProxyLocalToWorld.TransformPosition(Line.Key), ProxyLocalToWorld.TransformPosition(Line.Value),
						LineColor, SDPG_World, LineThickness);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			// bHiddenInGame이 켜져 있으므로 게임/PIE 뷰에서는 IsShown이 false가 된다.
			Result.bDrawRelevance = IsShown(View);
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = false;
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}

		virtual uint32 GetMemoryFootprint() const override
		{
			return sizeof(*this) + GetAllocatedSize();
		}

	private:

		TArray<TPair<FVector, FVector>> Lines;
	};
}

UCharonWaterNavCollisionComponent::UCharonWaterNavCollisionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	// 쿼리 콜리전은 켜되 모든 채널을 무시한다.
	// 콜리전이 켜져 있어야 엔진이 물리 데이터를 만들고, 내비메시 생성이 이 형상을 기본 경로로 가져간다.
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldStatic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetGenerateOverlapEvents(false);

	SetCastShadow(false);
	bHiddenInGame = true;
	bCanEverAffectNavigation = true;

	// 콜리전 아래 공간을 막힌 것으로 채워서 강바닥에 내비메시가 생기지 않게 한다.
	// 이 플래그는 GetNavigationData에서 부모 구현을 호출해야 내비게이션 시스템에 전달된다.
	bFillCollisionUnderneathForNavmesh = true;
}

void UCharonWaterNavCollisionComponent::SetNavConvexes(const TArray<TArray<FVector>>& WorldSpaceConvexes)
{
	if (NavBodySetup == nullptr)
	{
		NavBodySetup = NewObject<UBodySetup>(this, TEXT("NavBodySetup"));
		NavBodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
	}

	NavBodySetup->AggGeom.EmptyElements();

	const FTransform WorldToLocal = GetComponentTransform().Inverse();

	for (const TArray<FVector>& WorldVerts : WorldSpaceConvexes)
	{
		if (WorldVerts.Num() < 4)
		{
			continue;
		}

		FKConvexElem Convex;
		Convex.VertexData.Reserve(WorldVerts.Num());
		for (const FVector& WorldVert : WorldVerts)
		{
			Convex.VertexData.Add(WorldToLocal.TransformPosition(WorldVert));
		}
		Convex.UpdateElemBox();

		NavBodySetup->AggGeom.ConvexElems.Add(MoveTemp(Convex));
	}

	// 형상이 바뀌었으니 기존 물리 데이터를 버리고 새 형상으로 다시 쿠킹한 뒤, 물리 바디를 다시 만든다.
	NavBodySetup->InvalidatePhysicsData();
	NavBodySetup->CreatePhysicsMeshes();
	RecreatePhysicsState();
	UpdateBounds();

	// 형상이 바뀌었으니 씬 프록시를 다시 만들어 외곽선 표시를 갱신한다.
	MarkRenderStateDirty();

	// 처음 등록될 때는 형상이 없어서 '내비 무관'으로 판정·캐시됐을 수 있다.
	// 플래그를 껐다 켜서 판정을 다시 하고 내비 시스템에 재등록시킨다.
	SetCanEverAffectNavigation(false);
	SetCanEverAffectNavigation(true);

	UE_LOGF(LogCharon, Log, "UCharonWaterNavCollisionComponent::SetNavConvexes : Convex=%d, PhysicsBody=%ls",
		GetNavConvexCount(), BodyInstance.IsValidBodyInstance() ? TEXT("Valid") : TEXT("Invalid"));
}

int32 UCharonWaterNavCollisionComponent::GetNavConvexCount() const
{
	return NavBodySetup ? NavBodySetup->AggGeom.ConvexElems.Num() : 0;
}

FPrimitiveSceneProxy* UCharonWaterNavCollisionComponent::CreateSceneProxy()
{
	if (!bDrawNavCollision || GetNavConvexCount() == 0)
	{
		return nullptr;
	}

	return new CharonWaterNavCollision::FNavCollisionSceneProxy(this);
}

FBoxSphereBounds UCharonWaterNavCollisionComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (NavBodySetup == nullptr || NavBodySetup->AggGeom.ConvexElems.Num() == 0)
	{
		return FBoxSphereBounds(LocalToWorld.GetLocation(), FVector::ZeroVector, 0.f);
	}

	FBox LocalBox(ForceInit);
	for (const FKConvexElem& Convex : NavBodySetup->AggGeom.ConvexElems)
	{
		LocalBox += Convex.ElemBox;
	}

	return FBoxSphereBounds(LocalBox.TransformBy(LocalToWorld));
}

void UCharonWaterNavCollisionComponent::GetNavigationData(FNavigationRelevantData& Data) const
{
	// 부모 구현이 bFillCollisionUnderneathForNavmesh 같은 컴포넌트 내비 옵션을 Data에 실어준다.
	// 호출하지 않으면 해당 옵션들이 디테일에서 켜져 있어도 무시된다.
	Super::GetNavigationData(Data);

	if (!NavAreaClass || NavBodySetup == nullptr)
	{
		return;
	}

	// UWaterBodyComponent::GetNavigationData와 같은 방식으로 영역 모디파이어를 만든다.
	FCompositeNavModifier CompositeNavModifier;
	CompositeNavModifier.CreateAreaModifiers(this, NavAreaClass);
	for (FAreaNavModifier& AreaNavModifier : CompositeNavModifier.GetMutableAreas())
	{
		AreaNavModifier.SetExpandTopByCellHeight(true);
	}

	Data.Modifiers.Add(CompositeNavModifier);
}

bool UCharonWaterNavCollisionComponent::IsNavigationRelevant() const
{
	// 엔진 기본 구현은 Pawn/Vehicle 채널을 막는 콜리전만 내비게이션 대상으로 본다.
	// 이 컴포넌트는 모든 채널을 무시하므로 직접 판정한다.
	return CanEverAffectNavigation() && GetNavConvexCount() > 0;
}
