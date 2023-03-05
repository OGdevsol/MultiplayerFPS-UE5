// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "GameplayTagAssetInterface.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Evaluation/Blending/MovieSceneBlendType.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"


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
		FHitResult FireHit;
		WeaponTraceHit(Start,HitTarget,FireHit);
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(BlasterCharacter,Damage, InstigatorController,this, UDamageType::StaticClass());
				
			
		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles,FireHit.ImpactPoint,FireHit.ImpactNormal.Rotation());
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,HitSound,FireHit.ImpactPoint);
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),BeamParticles,SocketTransform); 
			if (ParticleComponent)
			{
				ParticleComponent->SetVectorParameter(FName("Target"),HitTarget);
			}
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),MuzzleFlash,SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,FireSound,GetActorLocation());
		}
	}
	
	
	
}
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart ,const FVector& HitTarget,  FHitResult& OutHit)
{
//	FHitResult FireHit;
	FVector End = bUseScatter ? TraceEndWithScatter(TraceStart,HitTarget)  : TraceStart + (HitTarget-TraceStart)*1.25;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(OutHit,TraceStart,End,ECollisionChannel::ECC_Visibility);
	}
	FVector BeamEnd = End;
	if (OutHit.bBlockingHit)
	{
		BeamEnd = OutHit.ImpactPoint;
	}
	if (BeamParticles)
	{
		UParticleSystemComponent* ParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(World,BeamParticles,TraceStart,FRotator::ZeroRotator,true); 
		if (ParticleComponent)
		{
			ParticleComponent->SetVectorParameter(FName("Target"),BeamEnd);
		}
	}
	
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalize = (HitTarget-TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() *FMath::FRandRange(0.f,SphereRadius);
	FVector EndLoc = SphereCenter+RandVec;
	FVector ToEndLoc=EndLoc-TraceStart;
	float  TL = TRACE_LENGTH;
	/*DrawDebugSphere(GetWorld(),SphereCenter,SphereRadius,12,FColor::Red,true);
	DrawDebugSphere(GetWorld(),EndLoc,SphereRadius,4.f,FColor::Green,true);
	DrawDebugLine(GetWorld(),TraceStart,FVector(TraceStart+ToEndLoc*TL/ToEndLoc.Size()),FColor::Cyan,true);*/
	
	return    FVector(TraceStart + ToEndLoc * TL / ToEndLoc.Size());
}



