// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "HitScanWeapon.h"
#include "GameplayTagAssetInterface.h"
#include "Particles/ParticleSystemComponent.h"


void AShotgun::Fire(const FVector& HitTarget)
{
	
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket -> GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
	
		TMap<ABlasterCharacter*,uint32> HitMap;
		for (uint32 i = 0; i<NumberOfPellets; i++ )
		{
			FHitResult FireHit;
			WeaponTraceHit(Start,HitTarget,FireHit);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter,1);
				}
			
			}
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,FireHit.ImpactPoint,FireHit.ImpactNormal.Rotation());
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this,HitSound,FireHit.ImpactPoint,.5f,FMath::FRandRange(-.5f,5.f));
				
			}
			

		}
		for (auto HitPair:HitMap)
		{
			if (InstigatorController)
			{
				if (HitPair.Key && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key,Damage*HitPair.Value, InstigatorController,this, UDamageType::StaticClass());
				}
			}
		}

		/*FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(FireHit,Start,End,ECollisionChannel::ECC_Visibility);
		}
		FVector BeamEnd = End;
		if (FireHit.bBlockingHit)
		{
			BeamEnd = FireHit.ImpactPoint;
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(BlasterCharacter,Damage, InstigatorController,this, UDamageType::StaticClass());
				
			
			}
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World,ImpactParticles,FireHit.ImpactPoint,FireHit.ImpactNormal.Rotation());
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this,HitSound,FireHit.ImpactPoint);
			}
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(World,BeamParticles,SocketTransform); 
			if (ParticleComponent)
			{
				ParticleComponent->SetVectorParameter(FName("Target"),BeamEnd);
			}
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World,MuzzleFlash,SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,FireSound,GetActorLocation());
		}*/
	}
}
