// Fill out your copyright notice in the Description page of Project Settings.


#include "CharonCharacterMovementComponent.h"

#include "Environment/Water/Buoyancy/SwimBuoyancyComponent.h"
#include "GameFramework/Character.h"


UCharonCharacterMovementComponent::UCharonCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SwimBuoyancyComponent = nullptr;
}

float UCharonCharacterMovementComponent::ImmersionDepth() const
{
	if(CharacterOwner)
	{		 		
		if(SwimBuoyancyComponent)
		{
			return SwimBuoyancyComponent->GetImmersionDepth();
		}
	}
	return 0.0f;
	
	// float depth = 0.f;
	//
	// if(CharacterOwner)
	// {
	// 	const float CollisionHalfHeight = CharacterOwner->GetSimpleCollisionHalfHeight();
	//
	// 	if ( (CollisionHalfHeight == 0.f) || (Buoyancy == 0.f) )
	// 	{
	// 		depth = 1.f;
	// 	}
	// 	else
	// 	{
	// 		//UBrushComponent* VolumeBrushComp = GetPhysicsVolume()->GetBrushComponent();
	// 		FHitResult Hit(1.f);
	// 		if ( CharacterOwner->GetOverlappingActors() )
	// 		{
	// 			const FVector TraceStart = UpdatedComponent->GetComponentLocation() + CollisionHalfHeight * -GetGravityDirection();
	// 			const FVector TraceEnd = UpdatedComponent->GetComponentLocation() - CollisionHalfHeight * -GetGravityDirection();
	//
	// 			FCollisionQueryParams NewTraceParams(SCENE_QUERY_STAT(ImmersionDepth), true);
	// 			VolumeBrushComp->LineTraceComponent( Hit, TraceStart, TraceEnd, NewTraceParams );
	// 		}
	//
	// 		depth = (Hit.Time == 1.f) ? 1.f : (1.f - Hit.Time);
	// 	}
	// }
	
	//return Super::ImmersionDepth();
}


USwimBuoyancyComponent* UCharonCharacterMovementComponent::GetSwimBuoyancyComponent()
{
	if(SwimBuoyancyComponent)
	{
		return SwimBuoyancyComponent;
	}
	return SwimBuoyancyComponent = CharacterOwner->FindComponentByClass<USwimBuoyancyComponent>();
}

void UCharonCharacterMovementComponent::PhysSwimming(float deltaTime, int32 Iterations)
{
	//Super::PhysSwimming(deltaTime, Iterations);
	
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	RestorePreAdditiveRootMotionVelocity();
	
	float NetFluidFriction  = 0.f;
	// ! GetSwimBuoyancyComponent로 SwimBuoyancyComponent초기화하는거니까 빼면 안됨!
	float Depth = GetSwimBuoyancyComponent() ? ImmersionDepth() : 0.0f;

	// Depth의 1초과분은 부력에만 적용. Depth는 다시 정규화.
	float NetBuoyancy = Buoyancy * Depth;
	Depth = FMath::Clamp(Depth, 0.f, 1.f);
	
	float OriginalAccelZ = GetGravitySpaceZ(Acceleration);
	bool bLimitedUpAccel = false;
	
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (GetGravitySpaceZ(Velocity) > 0.33f * MaxSwimSpeed) && (NetBuoyancy != 0.f))
	{
		//damp positive Z out of water
		SetGravitySpaceZ(Velocity, FMath::Max<FVector::FReal>(0.33f * MaxSwimSpeed, GetGravitySpaceZ(Velocity) * Depth*Depth));
	}
	//else if (Depth < 0.65f)
	
	// else if (Depth < 0.1f)
	// {
	// 	bLimitedUpAccel = (OriginalAccelZ > 0.f);
	// 	SetGravitySpaceZ(Acceleration, FMath::Min<FVector::FReal>(0.1f, OriginalAccelZ));
	// }
	
	Iterations++;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	bJustTeleported = false;
	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		
		//원본 - const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction * Depth;
		const float Friction = 4.f * 1.f * Depth;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());// bFluid = true
		
		
		
		////////////테스트중~~~~~~~~~
		FVector BuoyancyVel = (GetGravityZ() * deltaTime * (1.f - NetBuoyancy)) * -GetGravityDirection();
		// 수면을 따라 움직이게 Z 속도 조정
		if (GetSwimBuoyancyComponent())
		{
			
			float SlopeFollowMinHorizontalSpeed = 10.f;   // 이 속도 미만이면 경사 추종 안 함 (방향 노이즈 방지)
			
			float SlopeSampleTimeAhead = 0.08f;           // 몇 초 앞을 내다보고 샘플링할지
			
			float MinSlopeSampleDistance = 20.f;
			
			float MaxSlopeSampleDistance = 100.f;
			
			float MaxSlopeFollowVerticalSpeed = 600.f;    // 수직에 가까운 경사 대비 안전 클램프
			
			float SlopeFollowResponseSpeed = 5.f; 
			
			const FVector HorizontalVelocity(Velocity.X, Velocity.Y, 0.f);
			const float HorizontalSpeed = HorizontalVelocity.Size();
			
			
			// 너무 느리면(정지 수영 등) 방향이 불안정하니 건너뛰고 기존 부력만 적용
			if (HorizontalSpeed > SlopeFollowMinHorizontalSpeed)
			{
				const FVector TravelDir = HorizontalVelocity / HorizontalSpeed;

				// 속도가 빠를수록 더 멀리 앞을 내다봄
				const float SampleDistance = FMath::Clamp(HorizontalSpeed * SlopeSampleTimeAhead, MinSlopeSampleDistance, MaxSlopeSampleDistance);

				const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
				const FVector AheadLocation = CurrentLocation + TravelDir * SampleDistance;

				const float HeightHere = SwimBuoyancyComponent->GetWaterHeightAtLocation(CurrentLocation);
				const float HeightAhead = SwimBuoyancyComponent->GetWaterHeightAtLocation(AheadLocation);

				// 진행 방향 기울기 (dz/dx)
				Slope = (HeightAhead - HeightHere) / SampleDistance;

				// 연쇄법칙: 지금 속도로 이 기울기를 따라가려면 필요한 Z속도
				DesiredVelZ = Slope * HorizontalSpeed;

				// 부력도 한번 더 더해봐?
				//DesiredVelZ += BuoyancyVel.Z;
				
				// 폭포처럼 기울기가 거의 수직인 경우 극단값 방지
				DesiredVelZ = FMath::Clamp(DesiredVelZ, -MaxSlopeFollowVerticalSpeed, MaxSlopeFollowVerticalSpeed);

				// // 응답 속도도 현재 속력에 비례 -> 빠를수록 더 즉각적으로 반응
				// const float ResponseSpeed = SlopeFollowResponseSpeed * (HorizontalSpeed / FMath::Max(1.f, MaxSwimSpeed));
				//
				// const float CurrentVelZ = GetGravitySpaceZ(Velocity);
				// const float NewVelZ = FMath::FInterpTo(CurrentVelZ, DesiredVelZ, deltaTime, ResponseSpeed);
				// SetGravitySpaceZ(Velocity, NewVelZ);
				SetGravitySpaceZ(Velocity, DesiredVelZ);
			}	
		}
		
		Velocity += (GetGravityZ() * deltaTime * (1.f - NetBuoyancy)) * -GetGravityDirection();
	}
	
	ApplyRootMotionToVelocity(deltaTime);
	
	FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	const float remainingTime = deltaTime * Swim(Adjusted, Hit);
	
	//may have left water - if so, script might have set new physics mode
	if ( !IsSwimming() )
	{
		StartNewPhysics(remainingTime, Iterations);
		return;
	}
	
	if ( Hit.Time < 1.f && CharacterOwner)
	{
		HandleSwimmingWallHit(Hit, deltaTime);
		if (bLimitedUpAccel && (GetGravitySpaceZ(Velocity) >= 0.f))
		{
			// allow upward velocity at surface if against obstacle
			Velocity += OriginalAccelZ * deltaTime * -GetGravityDirection();
			Adjusted = Velocity * (1.f - Hit.Time)*deltaTime;
			Swim(Adjusted, Hit);
			if (!IsSwimming())
			{
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
		}
	
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = VelDir | GetGravityDirection();
	
		bool bSteppedUp = false;
		if( (FMath::Abs(GetGravitySpaceZ(Hit.ImpactNormal)) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			const float StepZ = GetGravitySpaceZ(UpdatedComponent->GetComponentLocation());
			const FVector RealVelocity = Velocity;
			SetGravitySpaceZ(Velocity, 1.f);	// HACK: since will be moving up, in case pawn leaves the water
			bSteppedUp = StepUp(GetGravityDirection(), Adjusted * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{
				//may have left water - if so, script might have set new physics mode
				if (!IsSwimming())
				{
					StartNewPhysics(remainingTime, Iterations);
					return;
				}
				SetGravitySpaceZ(OldLocation, GetGravitySpaceZ(UpdatedComponent->GetComponentLocation()) + (GetGravitySpaceZ(OldLocation) - StepZ));
			}
			Velocity = RealVelocity;
		}
	
		if (!bSteppedUp)
		{
			//adjust and try again
			HandleImpact(Hit, deltaTime, Adjusted);
			SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	}
	
	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bJustTeleported && ((deltaTime - remainingTime) > UE_KINDA_SMALL_NUMBER) && CharacterOwner )
	{
		// 원본 - const bool bWaterJump = !GetPhysicsVolume()->bWaterVolume;
		const bool bWaterJump = SwimBuoyancyComponent ? !SwimBuoyancyComponent->IsInWaterBody() : false;
		const FVector::FReal VelZ = GetGravitySpaceZ(Velocity);
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / (deltaTime - remainingTime);
		if (bWaterJump)
		{
			SetGravitySpaceZ(Velocity, VelZ);
		}
	}
	
	// 원본 - if ( !GetPhysicsVolume()->bWaterVolume && IsSwimming() )
	if ( (!SwimBuoyancyComponent || !SwimBuoyancyComponent->IsInWaterBody()) && IsSwimming() )
	{
		SetMovementMode(MOVE_Falling); //in case script didn't change it (w/ zone change)
	}
	
	//may have left water - if so, script might have set new physics mode
	if ( !IsSwimming() )
	{
		StartNewPhysics(remainingTime, Iterations);
	}
}
