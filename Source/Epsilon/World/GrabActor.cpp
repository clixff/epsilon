// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabActor.h"
#include "Kismet/GameplayStatics.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "../Core/EpsilonGameSession.h"


// Sets default values
AGrabActor::AGrabActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(StaticMesh);

	StaticMesh->SetSimulatePhysics(bShouldSimulatePhysics);
	StaticMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	StaticMesh->SetCastShadow(false);
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	StaticMesh->SetNotifyRigidBodyCollision(true);

	GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("Grab"));
	GrabComponent->SetupAttachment(StaticMesh);
}

// Called when the game starts or when spawned
void AGrabActor::BeginPlay()
{
	Super::BeginPlay();
	
	StaticMesh->SetSimulatePhysics(bShouldSimulatePhysics);
	//CollisionType = StaticMesh->GetCollisionEnabled();

	PhysicsSoundTimeout.Current = 0.0f;

	StaticMesh->OnComponentHit.AddDynamic(this, &AGrabActor::OnPhysicsHit);
}

// Called every frame
void AGrabActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Location = GetActorLocation();
	DeltaSpeed = (PrevLocation - Location).Length() * DeltaTime;

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
	StaticMesh->SetSimulatePhysics(false);

	//CollisionType = StaticMesh->GetCollisionEnabled();

	//StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGrabActor::OnUnGrab()
{
	bGrabbing = false;
	StaticMesh->SetCollisionEnabled(CollisionType);

	StaticMesh->SetSimulatePhysics(bShouldSimulatePhysics);
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
	if (PrevLocation == FVector::ZeroVector)
	{
		return;
	}

	if (!PhysicsSoundTimeout.IsEnded())
	{
		return;
	}

	if (!SpawnTimer.IsEnded())
	{
		return;
	}

	if (DeltaSpeed < 0.25f)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("[AGrabActor] Physics hit for \"%s\". Speed: %f"), *GetName(), DeltaSpeed);
	FVector Location = Hit.ImpactPoint;

	if (bDestroyOnDamage)
	{
		UE_LOG(LogTemp, Display, TEXT("[AGrabActor] Destroying physical object with speed %f"), DeltaSpeed);
		float Dist = FVector::Dist(PrevLocation, GetActorLocation());
		UE_LOG(LogTemp, Display, TEXT("[AGrabActor] Physics Timeout: %f. Dist: %f. Prev: %s, Loc: %s"), PhysicsSoundTimeout.Current, Dist, *PrevLocation.ToString(), *GetActorLocation().ToString());

		DestroyMesh();
	}

	auto* GameSession = UEpsilonGameSession::Get();

	if (!GameSession)
	{
		return;
	}
	
	USoundBase* Sound = GameSession->PhysicsHitSound;

	if (!Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, 1.0f, 1.0f);

	DrawDebugSphere(GetWorld(), Location, 20.0f, 8, FColor::Red, 0, 0.5f);

	PhysicsSoundTimeout.Reset();
}

void AGrabActor::DestroyMesh()
{
	if (!GeometryCollectionObject)
	{
		return;
	}

	FVector Velocity = GetVelocity();
	FVector Velocity2 = StaticMesh->GetPhysicsLinearVelocity();
	FVector AngularVel = StaticMesh->GetPhysicsAngularVelocityInDegrees();
	FTransform Transform = StaticMesh->GetComponentTransform();

	UE_LOG(LogTemp, Display, TEXT("[AGrabActor] Destroying vel: %s, vel2: %s"), *Velocity.ToString(), *Velocity2.ToString());


	Destroy();

	auto* DamagedActor = GetWorld()->SpawnActor<AGeometryCollectionActor>(Transform.GetLocation(), Transform.GetRotation().Rotator(), FActorSpawnParameters());

	DamagedActor->SetActorScale3D(Transform.GetScale3D());
	auto* Component = DamagedActor->GetGeometryCollectionComponent();
	Component->UnregisterComponent();

	Component->SetRestCollection(GeometryCollectionObject);
	Component->ObjectType = EObjectStateTypeEnum::Chaos_Object_Dynamic;
	Component->DamageThreshold = { 0.1f };
	Component->InitialLinearVelocity = Velocity2;
	Component->InitialVelocityType = EInitialVelocityTypeEnum::Chaos_Initial_Velocity_User_Defined;
	Component->InitialAngularVelocity = AngularVel;

	Component->RegisterComponent();

	Component->SetPhysicsLinearVelocity(Velocity2, false);

	Component->ComponentVelocity = Velocity2;

	UE_LOG(LogTemp, Display, TEXT("[AGrabActor] vel3: %s, vel4: %s"), *DamagedActor->GetVelocity().ToString(), *Component->GetPhysicsLinearVelocity().ToString());

}

