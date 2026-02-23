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
	float NetBuoyancy = Buoyancy * Depth;
	float OriginalAccelZ = GetGravitySpaceZ(Acceleration);
	bool bLimitedUpAccel = false;
	
	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (GetGravitySpaceZ(Velocity) > 0.33f * MaxSwimSpeed) && (NetBuoyancy != 0.f))
	{
		//damp positive Z out of water
		SetGravitySpaceZ(Velocity, FMath::Max<FVector::FReal>(0.33f * MaxSwimSpeed, GetGravitySpaceZ(Velocity) * Depth*Depth));
	}
	else if (Depth < 0.65f)
	{
		bLimitedUpAccel = (OriginalAccelZ > 0.f);
		SetGravitySpaceZ(Acceleration, FMath::Min<FVector::FReal>(0.1f, OriginalAccelZ));
	}
	
	Iterations++;
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	bJustTeleported = false;
	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		
		//원본 - const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction * Depth;
		const float Friction = 4.f * 1.f * Depth;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());// bFluid = true
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
