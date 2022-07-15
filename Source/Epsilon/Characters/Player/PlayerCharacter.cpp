// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "IXRTrackingSystem.h"
#include "Components/ArrowComponent.h"
#include "../../World/GrabActor.h"



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	Camera->SetupAttachment(GetRootComponent());
	Camera->bLockToHmd = false;
	//Camera->SetUsingAbsoluteLocation(true);
	//Camera->SetUsingAbsoluteRotation(true);

	MotionControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Left"));
	MotionControllerLeft->SetupAttachment(GetRootComponent());
	MotionControllerLeft->SetTrackingSource(EControllerHand::Left);

	MotionControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Right"));
	MotionControllerRight->SetupAttachment(GetRootComponent());
	MotionControllerRight->SetTrackingSource(EControllerHand::Right);

	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

	FistCollisionRight = CreateDefaultSubobject<UBoxComponent>(TEXT("FistRight"));
	FistCollisionRight->SetupAttachment(MotionControllerRight);
	FistCollisionRight->SetCollisionProfileName(TEXT("Fist"));
	FistCollisionRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FistCollisionLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("FistLeft"));
	FistCollisionLeft->SetupAttachment(MotionControllerLeft);
	FistCollisionLeft->SetCollisionProfileName(TEXT("Fist"));
	FistCollisionLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetArrowComponent()->SetHiddenInGame(false);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	bool bVREnabled = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (bVREnabled)
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	}

	UE_LOG(LogTemp, Display, TEXT("[APlayerPawn] bVREnabled: %d"), bVREnabled);

	Cast<APlayerController>(GetController())->ConsoleCommand(TEXT("stat unit"));
	Cast<APlayerController>(GetController())->ConsoleCommand(TEXT("stat fps"));

	Camera->SetWorldLocation(GetActorLocation());
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Update body mesh and collision when player moves in a room */
	//UpdateBodyPositionInVR();

	UpdateCachedControllerPosition(EHand::Right);
	UpdateCachedControllerPosition(EHand::Left);

	TraceFromFinger(EHand::Right);
	TraceFromFinger(EHand::Left);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCharacter::OnGrab(EHand Hand)
{
	if (Hand == EHand::Left && (GrabComponentLeft || GrabComponentFlyingLeft))
	{
		return;
	}
	else if (Hand == EHand::Right && (GrabComponentRight || GrabComponentFlyingRight))
	{
		return;
	}

	UPrimitiveComponent* HandComponent = nullptr;

	if (Hand == EHand::Left)
	{
		HandComponent = MotionControllerLeft;
	}
	else
	{
		HandComponent = MotionControllerRight;
	}

	if (!HandComponent)
	{
		return;
	}

	FVector Location = HandComponent->GetComponentLocation();

	float SphereRadius = 10.0f;

	FVector AdditionalOffset = FVector(0.0f, SphereRadius / 2.0f, 0.0f);

	if (Hand == EHand::Left)
	{
		AdditionalOffset *= -1.0f;
	}

	Location -= HandComponent->GetComponentRotation().RotateVector(AdditionalOffset);

	DrawDebugSphere(GetWorld(), Location, SphereRadius, 8, FColor(0, 255, 255, 255), false, 0.25f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(this);

	GetWorld()->SweepSingleByChannel(HitResult, Location, Location, FQuat(FRotator::ZeroRotator), ECollisionChannel::ECC_GameTraceChannel3, FCollisionShape::MakeSphere(SphereRadius), Params);

	UE_LOG(LogTemp, Display, TEXT("[APlayerCharacter] Grabbing: %d"), HitResult.bBlockingHit);

	if (!HitResult.bBlockingHit)
	{
		SetFistCollisionEnabled(Hand, true);
		return;
	}

	auto* GrabComponent = Cast<UGrabComponent>(HitResult.GetComponent());

	if (!GrabComponent)
	{
		return;
	}

	GrabComponent->OnGrab(Hand);
}

void APlayerCharacter::OnUnGrab(EHand Hand)
{
	SetFistCollisionEnabled(Hand, false);

	UGrabComponent* GrabComponent = nullptr;
	UMotionControllerComponent* HandComponent = nullptr;

	if (Hand == EHand::Left)
	{
		GrabComponent = GrabComponentLeft;
		HandComponent = MotionControllerLeft;
	}
	else
	{
		GrabComponent = GrabComponentRight;
		HandComponent = MotionControllerRight;
	}

	if (!GrabComponent)
	{
		return;
	}

	GrabComponent->OnUnGrab(Hand);

	if (HandComponent)
	{
		FVector Velocity = GetDeltaControllerPosition(Hand);

		UE_LOG(LogTemp, Display, TEXT("[APlayerCharacter] Velocity: %s"), *Velocity.ToString());
	}

}

void APlayerCharacter::SetFistCollisionEnabled(EHand Hand, bool bEnabled)
{
	auto* Fist = Hand == EHand::Right ? FistCollisionRight : FistCollisionLeft;

	Fist->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

	Fist->SetHiddenInGame(!bEnabled);

	Fist->SetGenerateOverlapEvents(bEnabled);
}

void APlayerCharacter::UpdateBodyPositionInVR()
{
	FVector HMDLocation;
	FRotator HMDRotation;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);

	FRotator BodyRotation = GetActorRotation();

	HMDRotation.Yaw += BodyRotation.Yaw;

	Camera->SetWorldLocationAndRotation(HMDLocation, HMDRotation);

	FVector CameraLocation = Camera->GetComponentLocation();
	FVector RightHandLocation = MotionControllerRight->GetComponentLocation();
	FVector LeftHandLocation = MotionControllerLeft->GetComponentLocation();

	FVector OldLocation = GetActorLocation();

	FVector NewLocation = CameraLocation;

	NewLocation.Z = OldLocation.Z;

	SetActorLocation(NewLocation);

	

	//Camera->SetWorldLocation(CameraLocation);
	//MotionControllerRight->SetWorldLocation(RightHandLocation);
	//MotionControllerLeft->SetWorldLocation(LeftHandLocation);
}

void APlayerCharacter::TraceFromFinger(EHand Hand)
{
	auto* ControllerRef = Hand == EHand::Right ? MotionControllerRight : MotionControllerLeft;
	FVector StartLocation = ControllerRef->GetComponentLocation();

	FRotator Rotation = ControllerRef->GetComponentRotation();

	StartLocation += Rotation.RotateVector(FingerTraceOffset);

	FVector DeltaLocation = FVector(0.0f, 0.0f, -1.0f) * 500.0f;

	DeltaLocation = Rotation.RotateVector(DeltaLocation);

	FVector EndLocation = StartLocation + DeltaLocation;
	FVector RealEndLocation = EndLocation;

	FHitResult HitResult;
	FCollisionQueryParams Params;

	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1, Params);

	if (HitResult.bBlockingHit)
	{
		RealEndLocation = HitResult.ImpactPoint;
		DrawDebugSphere(GetWorld(), RealEndLocation, 5.0f, 8, FColor(0, 255, 0, 255), false, -1.0f, 0, 0);
	}

	DrawDebugLine(GetWorld(), StartLocation, RealEndLocation, FColor(255, 255, 255, 128), false, -1.0f, 0, 0.5f);

	if (Hand == EHand::Right)
	{
		FingerTraceRight = RealEndLocation;
	}
	else
	{
		FingerTraceLeft = RealEndLocation;
	}
}

FVector APlayerCharacter::GetDeltaControllerPosition(EHand Hand)
{
	TArray<FVector>& Array = Hand == EHand::Right ? ControllerRightPositions : ControllerLeftPositions;

	if (Array.Num() <= 1)
	{
		return FVector::ZeroVector;
	}

	//return Array[Array.Num() - 1] - Array[0];

	FVector Delta;

	const float Scale = 15.0f;

	for (int32 i = 1; i < Array.Num(); i++)
	{
		FVector DeltaTemp = Array[i] - Array[i - 1];

		Delta += DeltaTemp;
	}

	return Delta * Scale;
}

void APlayerCharacter::OnAction(EHand Hand, bool bGrabPressed)
{
	UMotionControllerComponent* ControllerRef = Hand == EHand::Right ? MotionControllerRight : MotionControllerLeft;
	FVector& Location = Hand == EHand::Right ? FingerTraceRight : FingerTraceLeft;

	float SphereRadius = 50.0f;

	DrawDebugSphere(GetWorld(), Location, SphereRadius, 8, FColor(109, 89, 227, 255), false, 0.5f);

	if (!bGrabPressed)
	{
		return;
	}

	if (Hand == EHand::Left && (GrabComponentFlyingLeft || GrabComponentLeft))
	{
		return;
	}
	else if (Hand == EHand::Right && (GrabComponentFlyingRight || GrabComponentRight))
	{
		return;
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(this);

	GetWorld()->SweepMultiByChannel(HitResults, Location, Location, FQuat(FRotator::ZeroRotator), ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), Params);

	TArray<AGrabActor*> GrabActors;
	AGrabActor* GrabActor = nullptr;
	float MinDist = 0.0f;

	for (auto& HitResult : HitResults)
	{
		auto* Actor = Cast<AGrabActor>(HitResult.GetActor());

		if (!Actor)
		{
			continue;
		}

		float Dist = FVector::Dist(Location, Actor->GetActorLocation());

		if (!GrabActor || Dist < MinDist)
		{
			GrabActor = Actor;
			MinDist = Dist;
		}
	}

	if (!GrabActor)
	{
		return;
	}

	auto* GrabComponent = Cast<UGrabComponent>(GrabActor->GrabComponent);

	if (!GrabComponent)
	{
		return;
	}

	GrabComponent->FlyToController(ControllerRef);

	if (Hand == EHand::Right)
	{
		GrabComponentFlyingRight = GrabComponent;
	}
	else
	{
		GrabComponentFlyingLeft = GrabComponent;
	}
}


void APlayerCharacter::UpdateCachedControllerPosition(EHand Hand)
{
	TArray<FVector>& Array = Hand == EHand::Right ? ControllerRightPositions : ControllerLeftPositions;

	UMotionControllerComponent* HandComponent = Hand == EHand::Right ? MotionControllerRight : MotionControllerLeft;

	if (!HandComponent)
	{
		return;
	}

	FVector WorldLocation = HandComponent->GetComponentLocation();

	Array.Add(WorldLocation);

	if (Array.Num() > MaxControllerPositions)
	{
		Array.RemoveAt(0);
	}
}

