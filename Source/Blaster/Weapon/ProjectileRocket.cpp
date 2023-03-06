// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

#include "NiagaraFunctionLibrary.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"


AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RocketMovementComponent=CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity=true;
	RocketMovementComponent->SetIsReplicated(true);

	
}

void AProjectileRocket::Destroyed()
{
	
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this,&AProjectileRocket::OnHit);
	}
	SpawnTrailSystem();
	if (ProjectileLoop&&LoopingSoundAttenuation)
	{
		ProjectileloopComponent=UGameplayStatics::SpawnSoundAttached(ProjectileLoop,GetRootComponent(),FName(),GetActorLocation(),EAttachLocation::KeepWorldPosition,false,1.f,1.f,0.f,LoopingSoundAttenuation,(USoundConcurrency*) nullptr,false);
	}

}

void AProjectileRocket::DestroyTimerFinisher()
{
	Destroy();
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor==GetOwner())
	{
		return;
	}
	ExplodeDamage();
	StartDestroyTimer();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ImpactSound,GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate(true);
	}
	if (ProjectileloopComponent && ProjectileloopComponent->IsPlaying())
	{
		ProjectileloopComponent->Stop();
	}
}

