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
		FVector Velocity = PlayerCharacter->GetDeltaControllerPosition(Hand);
		//Velocity.Normalize();

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

		Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		Actor->bFlyingToController = true;
		//Actor->CollisionType = Actor->StaticMesh->GetCollisionEnabled();
		Actor->StaticMesh->SetSimulatePhysics(false);
		Actor->StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		FlyStartRotation = Actor->GetActorRotation();

		//Actor->SetActorRotation(FRotator::ZeroRotator);
	}

	FlyValue = 0.0f;
	FlyStart = GetComponentLocation();
	ControllerToFlyTo = Controller;
}

void UGrabComponent::GetTransformForAttach(EHand Hand, FVector& Location, FRotator& Rotation)
{
	/** Relative location (Component to actor) */
	FVector DeltaLocation = GetOwner()->GetActorLocation() - GetComponentLocation();

	float CollisionOffset = GetScaledBoxExtent().X;
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
	FlyValue += DeltaTime * 3.0f;

	FlyValue = FMath::Clamp(FlyValue, 0.0f, 1.0f);

	auto* Actor = Cast<AGrabActor>(GetOwner());
	auto* PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	EHand Hand = EHand::Right;

	if (PlayerCharacter->MotionControllerLeft == ControllerToFlyTo)
	{
		Hand = EHand::Left;
	}

	//FVector DeltaLocation = Actor->GetActorLocation() - GetComponentLocation();

	//float CollisionOffset = GetScaledBoxExtent().X;

	//if (Hand == EHand::Left)
	//{
	//	CollisionOffset *= -1.0f;
	//}

	//DeltaLocation.Y -= CollisionOffset;

	//DeltaLocation = ControllerToFlyTo->GetComponentRotation().RotateVector(DeltaLocation);

	FVector AttachLocation;
	FRotator AttachRotation;

	GetTransformForAttach(Hand, AttachLocation, AttachRotation);

	FVector FlyEnd = ControllerToFlyTo->GetComponentLocation() + AttachLocation;

	FVector NewLocation = FMath::Lerp(FlyStart, FlyEnd, FlyValue);

	Actor->SetActorLocation(NewLocation);

	FRotator StartRotation = FlyStartRotation;
	FRotator EndRotation = AttachRotation;

	FRotator NewRotation;

	NewRotation.Pitch = FMath::Lerp(StartRotation.Pitch, EndRotation.Pitch, FlyValue);
	NewRotation.Yaw = FMath::Lerp(StartRotation.Yaw, EndRotation.Yaw, FlyValue);
	NewRotation.Roll = FMath::Lerp(StartRotation.Roll, EndRotation.Roll, FlyValue);

	Actor->SetActorRotation(NewRotation);

	if (FlyValue >= 1.0f)
	{
		if (Actor)
		{
			Actor->bFlyingToController = false;
			Actor->StaticMesh->SetCollisionEnabled(Actor->CollisionType);
		}

		OnGrab(Hand);

		ControllerToFlyTo = nullptr;
		FlyValue = 0.0f;
		FlyStart = FVector::ZeroVector;
	}
}
