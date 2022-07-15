// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "DrawDebugHelpers.h"



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	Camera->SetupAttachment(GetRootComponent());

	MotionControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Left"));
	MotionControllerLeft->SetupAttachment(GetRootComponent());
	MotionControllerLeft->SetTrackingSource(EControllerHand::Left);

	MotionControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Right"));
	MotionControllerRight->SetupAttachment(GetRootComponent());
	MotionControllerRight->SetTrackingSource(EControllerHand::Right);
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
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCachedControllerPosition(EHand::Right);
	UpdateCachedControllerPosition(EHand::Left);

	TraceFromFinger();
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
		return;
	}

	auto* GrabComponent = Cast<UGrabComponent>(HitResult.GetComponent());

	if (!GrabComponent)
	{
		return;
	}

	GrabComponent->OnGrab(Hand);

	Cast<APlayerController>(GetController())->PlayHapticEffect(ControllerVibrationCurve, Hand == EHand::Right ? EControllerHand::Right : EControllerHand::Left, 1.0f, false);
}

void APlayerCharacter::OnUnGrab(EHand Hand)
{
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

void APlayerCharacter::TraceFromFinger()
{
	FVector StartLocation = MotionControllerRight->GetComponentLocation();

	FRotator Rotation = MotionControllerRight->GetComponentRotation();

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

	FingerTraceRight = RealEndLocation;
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

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(this);

	GetWorld()->SweepSingleByChannel(HitResult, Location, Location, FQuat(FRotator::ZeroRotator), ECollisionChannel::ECC_GameTraceChannel3, FCollisionShape::MakeSphere(SphereRadius), Params);


	if (!HitResult.bBlockingHit)
	{
		return;
	}

	auto* GrabComponent = Cast<UGrabComponent>(HitResult.GetComponent());

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

