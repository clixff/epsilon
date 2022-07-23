// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Main = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Main);
	Main->SetMobility(EComponentMobility::Static);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	Mesh->SetupAttachment(Main);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCastShadow(false);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetMassOverrideInKg(NAME_None, 10.0f);
	Mesh->SetCollisionProfileName(TEXT("Door"));
	Mesh->SetGenerateOverlapEvents(false);

	Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Frame"));
	Frame->SetupAttachment(Main);
	Frame->SetMobility(EComponentMobility::Static);
	Frame->SetGenerateOverlapEvents(false);

	Constraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Constraint"));
	Constraint->SetupAttachment(Main);
	Constraint->SetRelativeLocation(FVector(-81.0f, 0.0f, 200.0f));
	Constraint->ComponentName1.ComponentName = TEXT("Frame");
	Constraint->ComponentName2.ComponentName = TEXT("Door");

	Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 90.0f);
	Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 45.0f);
	Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 45.0f);

	Constraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	Constraint->SetAngularOrientationDrive(true, false);
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

