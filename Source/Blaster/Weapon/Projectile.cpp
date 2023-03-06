 #include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "Projectile_Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Blaster/Blaster.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetUpCollision(); //Set up the properties for collision
	
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(Tracer,CollisionBox,FName(),GetActorLocation(),GetActorRotation(),EAttachLocation::KeepWorldPosition);
	}
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this,&AProjectile::OnHit);
	}
	
}

void AProjectile::StartDestroyTimer()
{

	GetWorldTimerManager().SetTimer(DestroyTimer,this,&AProjectile::DestroyTimerFinisher,DestroyTime);
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
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	UE_LOG(LogTemp,Warning,TEXT("CollisionSetUpComplete"));
}

 void AProjectile::ExplodeDamage()
 {
	
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this,
				Damage,// Base Damage
				10.f, // Min Dmg
				GetActorLocation(), // Origin
				InnerRadius, // Dmg Inner Radius
				OuterRadius, // Dmg Outer Radius
				1.f, //Damage Falloff
				UDamageType::StaticClass(), //DamageTypeClass
				TArray<AActor*>(), // Actors to be ignored
				this , // Causer
				FiringController);//Instigator Controller
		}
	}
 }

 void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                         FVector NormalImpulse, const FHitResult& Hit)
{
	
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent =	UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem,GetRootComponent(),FName(),GetActorLocation(),GetActorRotation(),EAttachLocation::KeepWorldPosition,false);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::DestroyTimerFinisher()
{
	Destroy();
}
void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,ImpactSound,GetActorLocation());
	}
}
