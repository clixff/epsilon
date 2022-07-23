// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabActor.h"
#include "Kismet/GameplayStatics.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "../../Characters/Player/PlayerCharacter.h"
#include "../../Core/EpsilonGameSession.h"


// Sets default values
AGrabActor::AGrabActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(StaticMesh);

	SetSimulatePhysics(bShouldSimulatePhysics);
	StaticMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	StaticMesh->SetCastShadow(false);
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	StaticMesh->SetNotifyRigidBodyCollision(true);
	StaticMesh->SetCanEverAffectNavigation(false);

	GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("Grab"));
	GrabComponent->SetupAttachment(StaticMesh);
}

// Called when the game starts or when spawned
void AGrabActor::BeginPlay()
{
	Super::BeginPlay();
	
	SetSimulatePhysics(bShouldSimulatePhysics);
	//CollisionType = StaticMesh->GetCollisionEnabled();

	PhysicsSoundTimeout.Current = 0.0f;

	GetMeshComponent()->OnComponentHit.AddDynamic(this, &AGrabActor::OnPhysicsHit);
}

// Called every frame
void AGrabActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Location = GetActorLocation();
	DeltaSpeed = StaticMesh->GetComponentVelocity().Length();

	PrevLocation = Location;

	if (!PhysicsSoundTimeout.IsEnded())
	{
		PhysicsSoundTimeout.Add(DeltaTime);
	}

	if (!SpawnTimer.IsEnded())
	{
		SpawnTimer.Add(DeltaTime);
	}
}

void AGrabActor::OnGrab(EHand Hand)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	bGrabbing = true;
	SetSimulatePhysics(false);

	HandToAttach = Hand;

	//CollisionType = StaticMesh->GetCollisionEnabled();

	//StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGrabActor::OnUnGrab()
{
	bGrabbing = false;
	GetMeshComponent()->SetCollisionEnabled(CollisionType);

	SetSimulatePhysics(bShouldSimulatePhysics);
}

UGrabComponent* AGrabActor::GetNearestGrabComponent(FVector Location)
{
	TInlineComponentArray<UGrabComponent*> GrabComponents(this);
		
	GetComponents<UGrabComponent>(GrabComponents, true);

	if (GrabComponents.Num() < 2)
	{
		return GrabComponent;
	}

	UGrabComponent* Component = nullptr;
	float MinDist = 0.0f;

	for (auto* Comp : GrabComponents)
	{
		float Dist = FVector::Dist(Location, Comp->GetComponentLocation());

		if (!Component || Dist < MinDist)
		{
			MinDist = Dist;
			Component = Comp;
		}
	}

	return Component;
}

void AGrabActor::OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bGrabbing)
	{
		PlayAttachedVibration();
	}

	if (PrevLocation == FVector::ZeroVector)
	{
		return;
	}

	if (!SpawnTimer.IsEnded())
	{
		return;
	}

	if (DeltaSpeed < 400.0f)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[AGrabActor] Physics hit for \"%s\". Speed: %f"), *GetName(), DeltaSpeed);
	FVector Location = Hit.ImpactPoint;

	DamageObject();
		
	PlayPhysicsSound(Location);

	auto* GrabActorOther = Cast<AGrabActor>(OtherActor);

	if (GrabActorOther)
	{
		GrabActorOther->DamageObject();
		GrabActorOther->PlayPhysicsSound(Location);
	}
}

UMeshComponent* AGrabActor::GetMeshComponent()
{
	return StaticMesh;
}

void AGrabActor::SetSimulatePhysics(bool bSimulate)
{
	GetMeshComponent()->SetSimulatePhysics(bSimulate);
}

void AGrabActor::PlayPhysicsSound(FVector Location)
{
	if (!PhysicsSoundTimeout.IsEnded())
	{
		return;
	}

	USoundBase* Sound = HitSound;

	if (!Sound)
	{
		auto* GameSession = UEpsilonGameSession::Get();

		if (!GameSession)
		{
			return;
		}

		Sound = GameSession->PhysicsHitSound;
	}

	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, 1.0f, 1.0f);

	DrawDebugSphere(GetWorld(), Location, 20.0f, 8, FColor::Red, 0, 0.5f);

	PhysicsSoundTimeout.Reset();
}

void AGrabActor::RemoveAttachment()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	GetMeshComponent()->SetCollisionEnabled(CollisionType);

	SetSimulatePhysics(bShouldSimulatePhysics);

	bGrabbing = false;
	bFlyingToController = false;

	TInlineComponentArray<UGrabComponent*> GrabComponents(this);

	GetComponents<UGrabComponent>(GrabComponents, true);

	for (auto* GrabComponentRef : GrabComponents)
	{
		GrabComponentRef->bGrabbing = false;
		GrabComponentRef->ControllerToFlyTo = nullptr;
	}

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (HandToAttach == EHand::Right)
	{
		PlayerCharacter->GrabComponentRight = nullptr;
		PlayerCharacter->GrabComponentFlyingRight = nullptr;
	}
	else
	{
		PlayerCharacter->GrabComponentLeft = nullptr;
		PlayerCharacter->GrabComponentFlyingLeft = nullptr;
	}
}

void AGrabActor::DamageObject()
{
}

void AGrabActor::PlayAttachedVibration()
{
	if (!bGrabbing)
	{
		return;
	}

	auto* PlayerCharacter = Cast<APlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	if (!PlayerCharacter)
	{
		return;
	}

	PlayerCharacter->PlayControllerVibration(HandToAttach, true, 0.5f, EVibrationType::Physics);
}

