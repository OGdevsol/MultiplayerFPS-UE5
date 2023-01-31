// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SetUpCollision(); //Set up the properties for collision 
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectile::SetUpCollision()
{
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	UE_LOG(LogTemp,Warning,TEXT("CollisionSetUpComplete"));
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
