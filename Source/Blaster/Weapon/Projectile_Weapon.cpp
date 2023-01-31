// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectile_Weapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority()) return;
	
    const USkeletalMeshSocket* MuzzleFlashSocket =	GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (MuzzleFlashSocket)
	{
		 FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation(); //From muzzle to hot location from traceundercrosshair
		FRotator TargetRotation = ToTarget.Rotation();
		 if (ProjectileClass && InstigatorPawn)
		 {
		 	FActorSpawnParameters SpawnParams;
		 	SpawnParams.Owner=GetOwner();
		 	SpawnParams.Instigator=InstigatorPawn;
			 UWorld* World = GetWorld();
			 if (World)
			 {
				 World->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);
			 }
		 }
	}
}
