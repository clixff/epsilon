// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabActorDestroyable.h"
#include "../../Characters/Player/PlayerCharacter.h"
#include "../Chaos/GeometryCollectionActorDynamic.h"
#include "GeometryCollection/GeometryCollectionActor.h"

AGrabActorDestroyable::AGrabActorDestroyable()
{


}

AGrabActorDestroyable::~AGrabActorDestroyable()
{

}

void AGrabActorDestroyable::BeginPlay()
{
	Super::BeginPlay();
}

void AGrabActorDestroyable::DamageObject()
{
	if (!GeometryCollection)
	{
		return;
	}

	if (bGrabbing || bFlyingToController)
	{
		RemoveAttachment();
	}

	FTransform OldTransform = StaticMesh->GetComponentTransform();
	FVector OldVelocity = GetMeshComponent()->GetComponentVelocity();

	FVector BoundingSize = StaticMesh->GetStaticMesh()->GetBoundingBox().GetSize() * OldTransform.GetScale3D();

	float SphereRadius = FMath::Max3(BoundingSize.X, BoundingSize.Y, BoundingSize.Y) * 1.0f;

	Destroy();

	auto* DamagedActor = GetWorld()->SpawnActor<AGeometryCollectionActorDynamic>(OldTransform.GetLocation(), OldTransform.GetRotation().Rotator(), FActorSpawnParameters());

	auto* Component = DamagedActor->GetGeometryCollectionComponent();

	Component->UnregisterComponent();

	Component->SetRestCollection(GeometryCollection);
	Component->ObjectType = EObjectStateTypeEnum::Chaos_Object_Dynamic;
	Component->SetWorldScale3D(OldTransform.GetScale3D());
	Component->DamageThreshold = GeometryCollection->DamageThreshold;

	Component->RegisterComponent();

	DamagedActor->AddLinearVelocity(OldVelocity);
	DamagedActor->RemovePiecesTimer.Max = RemovePiecesTime;
	
	SphereRadius = DamageRadius;

	DamagedActor->ApplyDamageForce(SphereRadius);

	DrawDebugSphere(GetWorld(), OldTransform.GetLocation(), SphereRadius, 8, FColor::Yellow, false, 2.5f);
}
