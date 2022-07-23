// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "IXRTrackingSystem.h"
#include "PlayerPawnController.h"
#include "Components/ArrowComponent.h"



// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	Camera->SetupAttachment(VROrigin);

	MotionControllerLeft = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Left"));
	MotionControllerLeft->SetupAttachment(VROrigin);
	MotionControllerLeft->SetTrackingSource(EControllerHand::Left);

	MotionControllerRight = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController_Right"));
	MotionControllerRight->SetupAttachment(VROrigin);
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

	GrabPointMeshRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrabPointRight"));
	GrabPointMeshRight->SetupAttachment(VROrigin);
	GrabPointMeshRight->SetAbsolute(true, true, true);
	GrabPointMeshRight->SetVisibility(false);

	GrabPointMeshLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrabPointLeft"));
	GrabPointMeshLeft->SetupAttachment(VROrigin);
	GrabPointMeshLeft->SetAbsolute(true, true, true);
	GrabPointMeshLeft->SetVisibility(false);

	auto SetWidgetInteraction = [this](UWidgetInteractionComponent** ComponentRef, FName Name, UMotionControllerComponent* Parent)
	{
		(*ComponentRef) = CreateDefaultSubobject<UWidgetInteractionComponent>(Name);
		auto* Component = *ComponentRef;
		Component->SetupAttachment(Parent);
		Component->InteractionDistance = 500.0f;
		Component->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
		Component->SetRelativeLocation(FingerTraceOffset);
		Component->InteractionSource = EWidgetInteractionSource::World;
		Component->bShowDebug = true;
	};

	SetWidgetInteraction(&WidgetInteractionRight, TEXT("WidgetInteractionRight"), MotionControllerRight);
	SetWidgetInteraction(&WidgetInteractionLeft, TEXT("WidgetInteractionLeft"), MotionControllerLeft);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	bIsVREnabled = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

	if (bIsVREnabled)
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	}
	else
	{
		SetNonVRControls();
	}

	UE_LOG(LogTemp, Display, TEXT("[APlayerPawn] VR Enabled: %d"), bIsVREnabled);

	Cast<APlayerController>(GetController())->ConsoleCommand(TEXT("stat unit"));
	Cast<APlayerController>(GetController())->ConsoleCommand(TEXT("stat fps"));


	const float MaxVibrationTimeout = 0.5f;

	VibrationTimerRight.Max = MaxVibrationTimeout;
	VibrationTimerLeft.Max = MaxVibrationTimeout;

	VibrationTimerRight.Current = VibrationTimerRight.Max;
	VibrationTimerLeft.Current = VibrationTimerLeft.Max;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Update body mesh and collision when player moves in a room */
	UpdateBodyPositionInVR();

	UpdateCachedControllerPosition(EHand::Right, DeltaTime);
	UpdateCachedControllerPosition(EHand::Left, DeltaTime);

	TraceFromFinger(EHand::Right);
	TraceFromFinger(EHand::Left);

	UpdateGrabPoint(EHand::Right);
	UpdateGrabPoint(EHand::Left);

	if (!VibrationTimerLeft.IsEnded())
	{
		VibrationTimerLeft.Add(DeltaTime);
	}

	if (!VibrationTimerRight.IsEnded())
	{
		VibrationTimerRight.Add(DeltaTime);
	}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerCharacter::OnGrab(EHand Hand)
{
	if (Hand == EHand::Right)
	{
		if (GrabComponentRight || GrabComponentFlyingRight)
		{
			return;
		}
	}
	else
	{
		if (GrabComponentLeft || GrabComponentFlyingLeft)
		{
			return;
		}
	}

	auto* HandComponent = GetMotionController(Hand);

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

	//GrabComponent->OnGrab(Hand);
	GrabComponent->FlyToController(HandComponent, Hand);
}

void APlayerCharacter::OnUnGrab(EHand Hand)
{
	SetFistCollisionEnabled(Hand, false);
	DetachItemFromHand(Hand);
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
	FVector CameraLocation = Camera->GetComponentLocation();
	FVector ActorLocation = GetCapsuleComponent()->GetComponentLocation();

	FVector Delta = CameraLocation - ActorLocation;
	Delta.Z = 0.0f;

	AddActorWorldOffset(Delta);

	VROrigin->AddWorldOffset(Delta * -1.0f);
}

void APlayerCharacter::DetachItemFromHand(EHand Hand)
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

void APlayerCharacter::SpawnActor(EHand Hand)
{
	if (!GrabActorToSpawn)
	{
		return;
	}

	FVector& Location = Hand == EHand::Right ? FingerTraceRight : FingerTraceLeft;

	GetWorld()->SpawnActor<AGrabActor>(GrabActorToSpawn, Location, FRotator::ZeroRotator);
}

UMotionControllerComponent* APlayerCharacter::GetMotionController(EHand Hand)
{
	return Hand == EHand::Right ? MotionControllerRight : MotionControllerLeft;
}

void APlayerCharacter::PlayControllerVibration(EHand Hand, bool bIgnoreTimeout, float Scale, EVibrationType Type)
{
	FManualTimer& Timeout = Hand == EHand::Right ? VibrationTimerRight : VibrationTimerLeft;

	if (!bIgnoreTimeout && !Timeout.IsEnded())
	{
		return;
	}

	UHapticFeedbackEffect_Base* VibrationCurve = nullptr;

	switch (Type)
	{
	case EVibrationType::Grab:
		VibrationCurve = ControllerVibrationGrab;
		break;
	case EVibrationType::Physics:
		VibrationCurve = ControllerVibrationPhysics;
		break;
	}

	if (!VibrationCurve)
	{
		return;
	}

	Cast<APlayerController>(GetController())->PlayHapticEffect(VibrationCurve, Hand == EHand::Right ? EControllerHand::Right : EControllerHand::Left, Scale, false);

	Timeout.Current = 0.0f;
}

UWidgetInteractionComponent* APlayerCharacter::GetWidgetInteractionComponent(EHand Hand)
{
	return Hand == EHand::Right ? WidgetInteractionRight : WidgetInteractionLeft;
}

void APlayerCharacter::TriggerStateChanged(EHand Hand, bool bPressed)
{
	auto* Component = GetWidgetInteractionComponent(Hand);
	
	if (!Component)
	{
		return;
	}

	FKey LeftMouse = FKey(TEXT("LeftMouseButton"));

	if (bPressed)
	{
		Component->PressPointerKey(LeftMouse);
	}
	else
	{
		Component->ReleasePointerKey(LeftMouse);
	}
}

bool APlayerCharacter::IsVREnabled()
{
	return bIsVREnabled;
}

void APlayerCharacter::TraceFromFinger(EHand Hand)
{
	if (Hand == EHand::Right)
	{
		TraceFromFingerGrabRight = nullptr;
	}
	else
	{
		TraceFromFingerGrabLeft = nullptr;
	}

	auto* WidgetInteraction = GetWidgetInteractionComponent(Hand);

	auto* ControllerRef = GetMotionController(Hand);
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

	bool bShouldTrace = true;
	bool bBlockingHit = false;

	if (WidgetInteraction)
	{
		FHitResult WidgetHit = WidgetInteraction->GetLastHitResult();
		if (WidgetHit.bBlockingHit)
		{
			bShouldTrace = false;
			bBlockingHit = true;
			RealEndLocation = WidgetHit.ImpactPoint;
		}
	}

	if (bShouldTrace)
	{
		GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1, Params);

		bBlockingHit = HitResult.bBlockingHit;

		if (bBlockingHit)
		{
			RealEndLocation = HitResult.ImpactPoint;
		}
	}


	if (bBlockingHit)
	{
		DrawDebugSphere(GetWorld(), RealEndLocation, 5.0f, 8, FColor(0, 255, 0, 255), false, -1.0f, 0, 0);
	}

	DrawDebugLine(GetWorld(), StartLocation, RealEndLocation, FColor(255, 255, 255, 128), false, -1.0f, 0, 0.5f);

	bool bItemInHand = false;
	auto* PlayerPawnController = Cast<APlayerPawnController>(GetController());

	if (Hand == EHand::Right)
	{
		FingerTraceRight = RealEndLocation;
		bItemInHand = !!(GrabComponentRight || GrabComponentFlyingRight);
	}
	else
	{
		FingerTraceLeft = RealEndLocation;
		bItemInHand = !!(GrabComponentLeft || GrabComponentFlyingLeft);
	}

	if (bItemInHand)
	{
		return;
	}

	bool bIsGrabPressed = PlayerPawnController->IsGrabButtonPressed(Hand);

	if (!bIsGrabPressed)
	{
		return;
	}

	UpdateNearestItemForTeleport(Hand);
}

FVector APlayerCharacter::GetDeltaControllerPosition(EHand Hand)
{
	TArray<FVector>& Array = Hand == EHand::Right ? ControllerRightDeltas : ControllerLeftDeltas;

	if (Array.Num() <= 1)
	{
		return FVector::ZeroVector;
	}

	//return Array[Array.Num() - 1] - Array[0];

	FVector Delta;

	const float Scale = 0.3f;

	for (int32 i = 0; i < Array.Num(); i++)
	{
		FVector DeltaTemp = Array[i];

		Delta += DeltaTemp;
	}

	return Delta * Scale;
}

void APlayerCharacter::OnAction(EHand Hand, bool bGrabPressed)
{
	UMotionControllerComponent* ControllerRef = GetMotionController(Hand);
	FVector& Location = Hand == EHand::Right ? FingerTraceRight : FingerTraceLeft;


	if (bGrabPressed)
	{
		OnGrabTeleportAction(Hand);
	}
	else
	{
		auto* WidgetInteraction = GetWidgetInteractionComponent(Hand);

		if (WidgetInteraction)
		{
			FHitResult WidgetHit = WidgetInteraction->GetLastHitResult();
			if (WidgetHit.bBlockingHit)
			{
				return;
			}
		}

		SpawnActor(Hand);
		return;
	}
}


void APlayerCharacter::UpdateCachedControllerPosition(EHand Hand, float DeltaTime)
{
	TArray<FVector>& Array = Hand == EHand::Right ? ControllerRightDeltas : ControllerLeftDeltas;

	auto* HandComponent = GetMotionController(Hand);

	if (!HandComponent)
	{
		return;
	}

	FVector WorldLocation = HandComponent->GetComponentLocation();
	FVector OldLocation = Hand == EHand::Right ? ControllerRightLastTickLocation : ControllerLeftLastTickLocation;

	FVector Delta;
	Delta = (WorldLocation - OldLocation) / DeltaTime;

	if (Hand == EHand::Right)
	{
		ControllerRightLastTickLocation = WorldLocation;
	}
	else
	{
		ControllerLeftLastTickLocation = WorldLocation;
	}

	Array.Add(Delta);

	if (Array.Num() > MaxControllerPositions)
	{
		Array.RemoveAt(0);
	}
}

void APlayerCharacter::UpdateNearestItemForTeleport(EHand Hand)
{
	FVector Location = Hand == EHand::Right ? FingerTraceRight : FingerTraceLeft;
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	Params.AddIgnoredActor(this);

	float SphereRadius = 100.0f;

	GetWorld()->SweepMultiByChannel(HitResults, Location, Location, FQuat(FRotator::ZeroRotator), ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(SphereRadius), Params);

	UGrabComponent* NearestComponent = nullptr;
	float MinDist = 0.0f;

	for (auto& HitResult : HitResults)
	{
		auto* Actor = Cast<AGrabActor>(HitResult.GetActor());

		if (!Actor)
		{
			continue;
		}

		if (!Actor->bCanGrab || Actor->bGrabbing || Actor->bFlyingToController)
		{
			continue;
		}

		auto* Component = Actor->GetNearestGrabComponent(Location);

		float Dist = FVector::Dist(Location, Component->GetComponentLocation());

		if (!NearestComponent || Dist < MinDist)
		{
			NearestComponent = Component;
			MinDist = Dist;
		}
	}
	
	if (!NearestComponent)
	{
		return;
	}

	if (Hand == EHand::Right)
	{
		TraceFromFingerGrabRight = NearestComponent;
	}
	else
	{
		TraceFromFingerGrabLeft = NearestComponent;
	}
}

void APlayerCharacter::OnGrabTeleportAction(EHand Hand)
{
	UMotionControllerComponent* ControllerRef = GetMotionController(Hand);

	if (Hand == EHand::Left && (GrabComponentFlyingLeft || GrabComponentLeft))
	{
		return;
	}
	else if (Hand == EHand::Right && (GrabComponentFlyingRight || GrabComponentRight))
	{
		return;
	}

	auto* GrabComponent = Hand == EHand::Right ? TraceFromFingerGrabRight : TraceFromFingerGrabLeft;

	if (!GrabComponent)
	{
		return;
	}

	GrabComponent->FlyToController(ControllerRef, Hand);

	if (Hand == EHand::Right)
	{
		GrabComponentFlyingRight = GrabComponent;
	}
	else
	{
		GrabComponentFlyingLeft = GrabComponent;
	}
}

void APlayerCharacter::UpdateGrabPoint(EHand Hand)
{
	auto* GrabPoint = Hand == EHand::Right ? GrabPointMeshRight : GrabPointMeshLeft;

	if (!GrabPoint)
	{
		return;
	}

	auto* GrabComponent = Hand == EHand::Right ? TraceFromFingerGrabRight : TraceFromFingerGrabLeft;

	bool bOldVisibility = GrabPoint->IsVisible();
	bool bNewVisibility = GrabComponent != nullptr;

	FVector NewLocation = GrabComponent ? GrabComponent->GetComponentLocation() : FVector::ZeroVector;

	if (bOldVisibility != bNewVisibility)
	{
		GrabPoint->SetVisibility(bNewVisibility);
	}

	if (bNewVisibility)
	{
		GrabPoint->SetWorldLocation(NewLocation);
	}
}

void APlayerCharacter::SetNonVRControls()
{
	FRotator Rotation = FRotator(90.0f, 0.0f, 0.0f);
	MotionControllerRight->SetRelativeRotation(Rotation);
	MotionControllerLeft->SetRelativeRotation(Rotation);

	FVector HandLocation = FVector(41.0f, 24.0f, 30.0f);
	MotionControllerRight->SetRelativeLocation(HandLocation);
	MotionControllerLeft->SetRelativeLocation(HandLocation * FVector(1.0f, -1.0f, 1.0f));

	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
}

