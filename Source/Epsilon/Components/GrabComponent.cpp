// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabComponent.h"
#include "DrawDebugHelpers.h"
#include "../Characters/Player/PlayerCharacter.h"
#include "../World/GrabActor.h"


UGrabComponent::UGrabComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BoxExtent = FVector(10.0f);

	SetGenerateOverlapEvents(false);

	SetCollisionProfileName(TEXT("Grab"), false);

	PrimaryComponentTick.bCanEverTick = true;
}

void UGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DrawDebugSphere(GetWorld(), GetComponentLocation(), 5.0f, 8, FColor(128, 128, 128, 255));

	if (ControllerToFlyTo)
	{
		FlyToControllerTick(DeltaTime);
	}
}

void UGrabComponent::OnGrab(EHand Hand)
{
	if (bGrabbing)
	{
		return;
	}

	auto* Actor = Cast<AGrabActor>(GetOwner());

	if (Actor)
	{
		if (Actor->bGrabbing || Actor->bFlyingToController)
		{
			return;
		}

		Actor->OnGrab(Hand);
	}

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (PlayerCharacter)
	{
		if (Hand == EHand::Left)
		{
			PlayerCharacter->GrabComponentLeft = this;
			PlayerCharacter->GrabComponentFlyingLeft = nullptr;
		}
		else
		{
			PlayerCharacter->GrabComponentRight = this;
			PlayerCharacter->GrabComponentFlyingRight = nullptr;
		}
	}

	if (Actor && PlayerCharacter)
	{
		UPrimitiveComponent* HandComponent = nullptr;

		if (Hand == EHand::Left)
		{
			HandComponent = PlayerCharacter->MotionControllerLeft;
		}
		else
		{
			HandComponent = PlayerCharacter->MotionControllerRight;
		}

		PlayerCharacter->SetFistCollisionEnabled(Hand, false);

		if (HandComponent)
		{
			Actor->SetActorRotation(FRotator::ZeroRotator);

			FVector AttachLocation;
			FRotator AttachRotation;

			GetTransformForAttach(Hand, AttachLocation, AttachRotation);

			Actor->AttachToComponent(HandComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

			Actor->SetActorRelativeLocation(AttachLocation);
			Actor->SetActorRelativeRotation(AttachRotation);
		}
	}

	bGrabbing = true;

	GetWorld()->GetFirstPlayerController()->PlayHapticEffect(PlayerCharacter->ControllerVibrationCurve, Hand == EHand::Right ? EControllerHand::Right : EControllerHand::Left, 1.0f, false);
}

void UGrabComponent::OnUnGrab(EHand Hand)
{
	auto* PlayerCharacter =  Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (PlayerCharacter)
	{
		if (PlayerCharacter->GrabComponentRight == this)
		{
			PlayerCharacter->GrabComponentRight = nullptr;
		}

		if (PlayerCharacter->GrabComponentLeft == this)
		{
			PlayerCharacter->GrabComponentLeft = nullptr;
		}
	}
	
	auto* Actor = Cast<AGrabActor>(GetOwner());

	if (Actor)
	{
		Actor->OnUnGrab();

		Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	if (PlayerCharacter && Actor)
	{
		auto* ControllerRef = Hand == EHand::Right ? PlayerCharacter->MotionControllerRight : PlayerCharacter->MotionControllerLeft;


		FRotator HandRotation = ControllerRef->GetComponentRotation();
		FRotator ObjectRotation = Actor->GetActorRotation();

		FVector Velocity = PlayerCharacter->GetDeltaControllerPosition(Hand);

		Actor->StaticMesh->AddImpulse(Velocity, NAME_None, true);
	}

	bGrabbing = false;
}

void UGrabComponent::FlyToController(UPrimitiveComponent* Controller)
{
	if (bGrabbing)
	{
		return;
	}

	auto* Actor = Cast<AGrabActor>(GetOwner());


	if (Actor)
	{
		if (Actor->bGrabbing)
		{
			return;
		}

		FlyStartLocation = Actor->GetActorLocation();

		Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		Actor->bFlyingToController = true;
		Actor->StaticMesh->SetSimulatePhysics(false);
		Actor->StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Actor->SetActorRotation(FRotator::ZeroRotator);

		FlyStartRotation = Actor->GetActorRotation();
	}

	ControllerToFlyTo = Controller;

	float Dist = FVector::Dist(GetComponentLocation(), ControllerToFlyTo->GetComponentLocation());

	float MinTime = 0.1f;

	float MaxDist = 350.0f;
	float MaxTime = 0.75f;

	Dist = FMath::Clamp(Dist, 0.0f, MaxDist);

	FlyTimer.Current = 0.0f;
	FlyTimer.Max = FMath::Lerp(MinTime, MaxTime, Dist / MaxDist);
}

void UGrabComponent::GetTransformForAttach(EHand Hand, FVector& Location, FRotator& Rotation)
{
	/** Relative location (Component to actor) */
	FVector DeltaLocation = GetOwner()->GetActorLocation() - GetComponentLocation();

	float CollisionOffset = GetScaledBoxExtent().Y;
	FRotator ActorRelativeRotation = FRotator(-90.0f, 0.0f, 0.0f);

	if (Hand == EHand::Left)
	{
		CollisionOffset *= -1.0f;
		ActorRelativeRotation.Yaw = 180.0f;
		ActorRelativeRotation.Pitch = 90.0f;
	}

	DeltaLocation = ActorRelativeRotation.RotateVector(DeltaLocation);

	DeltaLocation.Y -= CollisionOffset;

	Location = DeltaLocation;
	Rotation = ActorRelativeRotation;
}

void UGrabComponent::FlyToControllerTick(float DeltaTime)
{
	FlyTimer.Add(DeltaTime);

	float FlyValue = FMath::Clamp(FlyTimer.Current / FlyTimer.Max, 0.0f, 1.0f);

	auto* Actor = Cast<AGrabActor>(GetOwner());
	auto* PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	EHand Hand = EHand::Right;

	if (PlayerCharacter->MotionControllerLeft == ControllerToFlyTo)
	{
		Hand = EHand::Left;
	}

	FVector FlyLocationStart = FlyStartLocation;
	FVector FlyLocationEnd;
	FVector FlyLocation;

	FRotator FlyRotationStart = FlyStartRotation;
	FRotator FlyRotationEnd;
	FRotator FlyRotation;

	Actor->SetActorRotation(FRotator::ZeroRotator);

	/** Object location in relative space */
	FVector AttachLocation;
	/** Object rotation in relative space */
	FRotator AttachRotation;

	GetTransformForAttach(Hand, AttachLocation, AttachRotation);

	Actor->AttachToComponent(ControllerToFlyTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	Actor->SetActorRelativeLocation(AttachLocation);
	Actor->SetActorRelativeRotation(AttachRotation);

	/** Object location in world space */
	FlyLocationEnd = Actor->GetActorLocation();
	/** Object rotation in world space */
	FlyRotationEnd = Actor->GetActorRotation();

	Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	FlyLocation = FMath::Lerp(FlyLocationStart, FlyLocationEnd, FlyValue);
	FlyRotation = FMath::Lerp(FlyRotationStart, FlyRotationEnd, FlyValue);

	Actor->SetActorLocation(FlyLocation);
	Actor->SetActorRotation(FlyRotation);

	if (FlyValue >= 1.0f)
	{
		if (Actor)
		{
			Actor->bFlyingToController = false;
			Actor->StaticMesh->SetCollisionEnabled(Actor->CollisionType);
		}

		OnGrab(Hand);

		ControllerToFlyTo = nullptr;
		FlyStartLocation = FVector::ZeroVector;
		FlyStartRotation = FRotator::ZeroRotator;
		FlyTimer.Reset();
	}
}
