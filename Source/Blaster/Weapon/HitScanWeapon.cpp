// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "GameplayTagAssetInterface.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Evaluation/Blending/MovieSceneBlendType.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket -> GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget-Start)*1.25;

		FHitResult FireHit;
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
		}
	}
}

