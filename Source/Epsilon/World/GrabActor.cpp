// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabActor.h"

// Sets default values
AGrabActor::AGrabActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(StaticMesh);

	StaticMesh->SetSimulatePhysics(bShouldSimulatePhysics);
	StaticMesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("Grab"));
	GrabComponent->SetupAttachment(StaticMesh);
}

// Called when the game starts or when spawned
void AGrabActor::BeginPlay()
{
	Super::BeginPlay();
	
	StaticMesh->SetSimulatePhysics(bShouldSimulatePhysics);
	CollisionType = StaticMesh->GetCollisionEnabled();

}

// Called every frame
void AGrabActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrabActor::OnGrab(EHand Hand)
{
	bGrabbing = true;
	StaticMesh->SetSimulatePhysics(false);

	CollisionType = StaticMesh->GetCollisionEnabled();

	//StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGrabActor::OnUnGrab()
{
	bGrabbing = false;
	StaticMesh->SetSimulatePhysics(bShouldSimulatePhysics);
	
	StaticMesh->SetCollisionEnabled(CollisionType);
}

