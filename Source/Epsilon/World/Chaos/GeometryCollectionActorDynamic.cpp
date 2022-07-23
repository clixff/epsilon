// Fill out your copyright notice in the Description page of Project Settings.


#include "GeometryCollectionActorDynamic.h"
#include "GeometryCollection/GeometryCollectionComponent.h"


AGeometryCollectionActorDynamic::AGeometryCollectionActorDynamic()
{
	UniformVector = CreateDefaultSubobject<UUniformVector>(TEXT("UniformVector"));

	Field = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("Field"));
	Field->SetupAttachment(GetRootComponent());

	GetGeometryCollectionComponent()->SetCastShadow(false);
	GetGeometryCollectionComponent()->SetCanEverAffectNavigation(false);
}

AGeometryCollectionActorDynamic::~AGeometryCollectionActorDynamic()
{

}

void AGeometryCollectionActorDynamic::AddLinearVelocity(FVector Velocity, float Scale)
{
	UniformVector->SetUniformVector(Scale, Velocity);

	GetGeometryCollectionComponent()->ApplyPhysicsField(true, EGeometryCollectionPhysicsTypeEnum::Chaos_LinearVelocity, nullptr, UniformVector);
}

void AGeometryCollectionActorDynamic::AddLinearVelocityAfterSpawn(FVector Velocity)
{
	VelocityToAdd = Velocity;
}

void AGeometryCollectionActorDynamic::ApplyDamageForce(float Radius)
{
	Field->ApplyStrainField(true, GetActorLocation(), Radius, 10000.0f, 1);
}

void AGeometryCollectionActorDynamic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!RemovePiecesTimer.IsEnded())
	{
		RemovePiecesTimer.Add(DeltaTime);

		if (RemovePiecesTimer.IsEnded())
		{
			Destroy();
		}
	}
}

void AGeometryCollectionActorDynamic::BeginPlay()
{
	Super::BeginPlay();

	GetGeometryCollectionComponent()->SetCollisionProfileName(TEXT("PhysicsActor"));

	if (VelocityToAdd != FVector::ZeroVector)
	{
		AddLinearVelocity(VelocityToAdd, 1.0f);
	}
}
